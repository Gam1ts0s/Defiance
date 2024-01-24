// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefianceCharacter.h"
#include "Animation/BaseAnimInstance_ABP.h"
#include "BasicSupportLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "MyInputConfigData.h"

//////////////////////////////////////////////////////////////////////////
// ADefianceCharacter

ADefianceCharacter::ADefianceCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	// set our turn rate for input
	TurnRateGamepad = 100.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;
	GetCharacterMovement()->SetCrouchedHalfHeight(CapsuleHalfHeightCrouched);
	GetCharacterMovement()->NavAgentProps.bCanCrouch = bCanCrouch;
	GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create Target Lock Sphere
	TargetLockSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TargetLockSphere"));
	TargetLockSphere->SetupAttachment(RootComponent);
	TargetLockSphere->SetSphereRadius(2000.f);
	TargetLockSphere->SetCollisionProfileName(FName("TargetLock"));

	// Create Target Lock Widget
	TargetLockWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetLockWidget"));
	TargetLockWidget->SetupAttachment(GetMesh());
	TargetLockWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	TargetLockWidget->Activate();
	TargetLockWidget->SetVisibility(false);


	// Create timeline components
	CameraRotationTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraRotationTimeline"));
	CameraRotationTimelineUpdate.BindUFunction(this, FName("OnCameraRotationTimelineUpdate"));
	CameraRotationTimelineEnd.BindUFunction(this, FName("OnCameraRotationTimelineEnd"));

	// Get pointers to the curves
	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve1(TEXT("CurveFloat'/Game/ThirdPerson/Curves/LinearCurve.LinearCurve'"));
	check(Curve1.Succeeded());
	LinearCurve = Curve1.Object;


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ADefianceCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate Target Lock Variables
	DOREPLIFETIME(ADefianceCharacter, bIsLockedOnTarget);
	DOREPLIFETIME(ADefianceCharacter, LockableTargets);
	DOREPLIFETIME(ADefianceCharacter, LockedOnTarget);
	
	//Replicate Movement Essential Variables
	DOREPLIFETIME(ADefianceCharacter, bUseDirectionalMovement);
	DOREPLIFETIME(ADefianceCharacter, MovementStance);
	DOREPLIFETIME(ADefianceCharacter, bCanSprint);
	DOREPLIFETIME(ADefianceCharacter, bIsSprinting);
	DOREPLIFETIME(ADefianceCharacter, bCanCrouch);
	DOREPLIFETIME(ADefianceCharacter, bIsCrouching);

	//Replicate Dodge & Roll Variables
	DOREPLIFETIME(ADefianceCharacter, DodgeRollAngle);
	DOREPLIFETIME(ADefianceCharacter, bCanDodge);
	DOREPLIFETIME(ADefianceCharacter, bIsDodging);
	DOREPLIFETIME(ADefianceCharacter, bCanRoll);
	DOREPLIFETIME(ADefianceCharacter, bIsRolling);
}


void ADefianceCharacter::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = Cast<UBaseAnimInstance_ABP>(GetMesh()->GetAnimInstance());
	if (AnimInstance == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("ADefianceCharacter [BeginPlay]: The animation instance has not been set."))
		return;
	}
	
	//Bind events with custom functions
	TargetLockSphere->OnComponentBeginOverlap.AddDynamic(this, &ADefianceCharacter::OnTargetLockSphereBeginOverlap);
	TargetLockSphere->OnComponentEndOverlap.AddDynamic(this, &ADefianceCharacter::OnTargetLockSphereEndOverlap);

	//Prepare Timelines
	if (LinearCurve != nullptr)
	{
		//CameraRotationTimeline
		CameraRotationTimeline->AddInterpFloat(LinearCurve, CameraRotationTimelineUpdate, FName("Alpha"));
		CameraRotationTimeline->SetTimelineFinishedFunc(CameraRotationTimelineEnd);
		CameraRotationTimeline->SetLooping(false);
		CameraRotationTimeline->SetIgnoreTimeDilation(false);
	}
}


void ADefianceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// To be executed only on owning Client
	//if (GEngine->GetNetMode(GetWorld()) == NM_Client)

	bool OwnedLocally = IsOwnedBy(UGameplayStatics::GetPlayerController(this, 0));
	if(OwnedLocally)
	{
		if (bIsLockedOnTarget)
		{
			if (!CameraRotationTimeline->IsPlaying())
			{
				// Orient Camera towards LockedOnTarget
				RotateCamera();
			}

			//Rotates the controller when locked on to target to follow camera orientation
			if (Controller != nullptr)
				Controller->SetControlRotation(FollowCamera->GetComponentRotation());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADefianceCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Get the player controller
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	// Get the local player subsystem
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	// Clear out existing mapping, and add our mapping
	EnhancedInputSubsystem->ClearAllMappings();
	EnhancedInputSubsystem->AddMappingContext(InputMapping, 0);

	// Get the EnhancedInputComponent
	check(PlayerInputComponent);
	UEnhancedInputComponent* PlayerEnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	// Set up gameplay key bindings
	PlayerEnhancedInput->BindAction(InputConfigData->InputMove, ETriggerEvent::Triggered, this, &ADefianceCharacter::Move);
	PlayerEnhancedInput->BindAction(InputConfigData->InputLook, ETriggerEvent::Triggered, this, &ADefianceCharacter::Look);
	PlayerEnhancedInput->BindAction(InputConfigData->InputLookStick, ETriggerEvent::Triggered, this, &ADefianceCharacter::LookStick);

	PlayerEnhancedInput->BindAction(InputConfigData->InputJump, ETriggerEvent::Started, this, &ADefianceCharacter::Jump);
	PlayerEnhancedInput->BindAction(InputConfigData->InputJump, ETriggerEvent::Completed, this, &ADefianceCharacter::StopJumping);
	PlayerEnhancedInput->BindAction(InputConfigData->InputSprintDodge, ETriggerEvent::Started, this, &ADefianceCharacter::DodgeRoll);
	PlayerEnhancedInput->BindAction(InputConfigData->InputSprintDodge, ETriggerEvent::Completed, this, &ADefianceCharacter::SR_EndSprint);
	PlayerEnhancedInput->BindAction(InputConfigData->InputCrouch, ETriggerEvent::Triggered, this, &ADefianceCharacter::SR_ToggleCrouch);

	PlayerEnhancedInput->BindAction(InputConfigData->InputTargetLock, ETriggerEvent::Triggered, this, &ADefianceCharacter::ToggleTargetLock);
}


void ADefianceCharacter::Jump()
{
	if (CanJump())
	{
		if (bIsCrouching)
		{
			SR_ToggleCrouch();
		}
	}

	Super::Jump();
}

void ADefianceCharacter::StopJumping()
{
	Super::StopJumping();
}

void ADefianceCharacter::DodgeRoll()
{
	// Get the possible dodge/roll angle in case of directional movement
	FVector MovementVector = GetPlayerInputMovementDirectionXY();
	FRotator MovementRotationAroundPlayer = GetTransform().InverseTransformVector(MovementVector).Rotation();
	float Angle = MovementRotationAroundPlayer.Yaw;
	SR_UpdateDodgeRollAngle(Angle);

	// Call DodgeRoll action on Server
	SR_DodgeRoll();
}

void ADefianceCharacter::SR_UpdateDodgeRollAngle_Implementation(float Angle)
{
	DodgeRollAngle = Angle;
}


void ADefianceCharacter::SR_DodgeRoll_Implementation()
{
	//Check if the character can and should Dodge ...
	if (bCanDodge && !bIsDodging && !bIsRolling && !GetCharacterMovement()->IsFalling())
	{
		if (bIsCrouching)
		{
			SR_ToggleCrouch();
		}
		SR_Dodge();
	}
	//... or can and should Roll
	else if (bCanRoll && !bIsRolling && !GetCharacterMovement()->IsFalling())
	{
		if (bIsCrouching)
		{
			SR_ToggleCrouch();
		}
		SR_Roll();
	}

	//Activate Sprinting until the button is released
	if (bCanSprint)
	{
		SR_StartSprint();
	}
}


void ADefianceCharacter::SR_Dodge_Implementation()
{
	bIsDodging = true;
	OnRep_IsDodging();
}

void ADefianceCharacter::OnRep_IsDodging()
{
	if (bIsDodging)
	{
		if (AnimInstance != nullptr)
		{
			if (bUseDirectionalMovement)
			{
				AnimInstance->PlayDirectionalMontage(DodgeRollAngle, AnimInstance->DodgeAnimMontage);
			}
			else
			{
				if (AnimInstance->DodgeAnimMontage.Contains(EDetailedDirection::Forward))
				{
					AnimInstance->Montage_Play(AnimInstance->DodgeAnimMontage[EDetailedDirection::Forward]);
				}
			}
		}
	}
}

void ADefianceCharacter::SR_Roll_Implementation()
{
	bIsRolling = true;
	OnRep_IsRolling();
}

void ADefianceCharacter::OnRep_IsRolling()
{
	if (bIsRolling)
	{
		if (AnimInstance != nullptr)
		{
			if (bUseDirectionalMovement)
			{
				AnimInstance->PlayDirectionalMontage(DodgeRollAngle, AnimInstance->RollAnimMontage);
			}
			else
			{
				if (AnimInstance->RollAnimMontage.Contains(EDetailedDirection::Forward))
				{
					AnimInstance->Montage_Play(AnimInstance->RollAnimMontage[EDetailedDirection::Forward]);
				}
			}
		}
	}
}


void ADefianceCharacter::SR_StartSprint_Implementation()
{
	bIsSprinting = true;
	OnRep_IsSprinting();
}

void ADefianceCharacter::SR_EndSprint_Implementation()
{
	bIsSprinting = false;
	OnRep_IsSprinting();
}

void ADefianceCharacter::OnRep_IsSprinting()
{
	if (bIsSprinting) {
		MovementStance = EMovementStance::Sprinting;
		GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;

		//If pawn currently uses directional movement change to non-directional movement
		if (bUseDirectionalMovement)
		{
			bUseControllerRotationYaw = false;
			GetCharacterMovement()->bOrientRotationToMovement = true;
		}
	}
	else
	{
		MovementStance = EMovementStance::Running;
		GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;

		//If character is locked on target when sprint ends then change to directional movement
		if (bUseDirectionalMovement)
		{
			bUseControllerRotationYaw = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}
	}
}


void ADefianceCharacter::SR_ToggleCrouch_Implementation()
{
	if (!bCanCrouch || GetCharacterMovement()->IsFalling() || bIsSprinting || bIsDodging || bIsRolling)
		return;
	else
	{
		if (!bIsCrouching)
		{
			bIsCrouching = true;
			OnRep_IsCrouching();
		}
		else
		{
			bIsCrouching = false;
			OnRep_IsCrouching();
		}
	}
}

void ADefianceCharacter::OnRep_IsCrouching()
{
	if (bIsCrouching)
	{
		MovementStance = EMovementStance::Crouched;
		Crouch();
		CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, (CapsuleHalfHeight-CapsuleHalfHeightCrouched)));
	}
	else
	{
		MovementStance = EMovementStance::Running;
		UnCrouch();
		CameraBoom->SetRelativeLocation(FVector::Zero());
	}
}

void ADefianceCharacter::Move(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, Controller->GetControlRotation().Yaw, 0);

		// Forward/Backward movement
		if (MoveValue.Y != 0.f)
		{
			// find out which way is forward
			FRotator Rotation;
			if (!bIsLockedOnTarget)
			{
				Rotation = Controller->GetControlRotation();
			}
			else
			{
				Rotation = FollowCamera->GetComponentRotation();
			}

			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Get forward vector
			//const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, MoveValue.Y);
		}

		// Right/Left movement
		if (MoveValue.X != 0.f)
		{
			// find out which way is right
			FRotator Rotation;
			if (!bIsLockedOnTarget)
			{
				Rotation = Controller->GetControlRotation();
			}
			else
			{
				Rotation = FollowCamera->GetComponentRotation();
			}
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Get right vector
			//const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, MoveValue.X);
		}
	}
}

void ADefianceCharacter::Look(const FInputActionValue& Value)
{
	if (!bIsLockedOnTarget)
	{
		if (Controller != nullptr)
		{
			const FVector2D LookValue = Value.Get<FVector2D>();

			if (LookValue.X != 0.f)
			{
				AddControllerYawInput(LookValue.X);
			}

			if (LookValue.Y != 0.f)
			{
				AddControllerPitchInput(LookValue.Y);
			}
		}
	}
}

void ADefianceCharacter::LookStick(const FInputActionValue& Value)
{
	if (!bIsLockedOnTarget)
	{
		if (Controller != nullptr)
		{
			const FVector2D LookValue = Value.Get<FVector2D>();

			if (LookValue.X != 0.f)
			{
				AddControllerYawInput(LookValue.X * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
			}

			if (LookValue.Y != 0.f)
			{
				AddControllerPitchInput(-1.0 * LookValue.Y * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
			}
		}
	}
}

void ADefianceCharacter::ToggleTargetLock()
{
	SR_ToggleTargetLock();
}


void ADefianceCharacter::SR_ToggleTargetLock_Implementation()
{
	if (!bIsLockedOnTarget) 
	{
		LockedOnTarget = SelectTargetToLockOn();
		if (LockedOnTarget != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("ADefianceCharacter [ToggleTargetLock]: Locking on %s."), *LockedOnTarget->GetName())

			// Play Camera rotation Timeline for client
			CL_PointCameraToTarget(LockedOnTarget);
			
			bIsLockedOnTarget = true;
			OnRep_IsLockedOnTarget();
		}
		else
		{
			CL_ResetCamera();
		}

		// Make TargetLockWidget visible on LockedOnTarget for the client
		CL_ToggleTargetLockWidget(LockedOnTarget);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ADefianceCharacter [ToggleTargetLock]: Unlocking from %s. "), *LockedOnTarget->GetName())

		// Make TargetLockWidget invisible on LockedOnTarget for the client
		CL_ToggleTargetLockWidget(LockedOnTarget);

		CL_ResetCamera();
		
		bIsLockedOnTarget = false;
		OnRep_IsLockedOnTarget();
		LockedOnTarget = nullptr;
	}
}

void ADefianceCharacter::CL_ToggleTargetLockWidget_Implementation(ADefianceCharacter* Target)
{
	if (Target == nullptr)
		return;

	if (Target->TargetLockWidget->IsVisible())
	{
		Target->TargetLockWidget->SetVisibility(false);
	}
	else
	{
		Target->TargetLockWidget->SetVisibility(true);
	}
}


void ADefianceCharacter::OnRep_IsLockedOnTarget()
{
	if (bIsLockedOnTarget)
	{
		if (!bIsSprinting)
		{
			bUseControllerRotationYaw = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}

		// Enable directional movement
		bUseDirectionalMovement = true;

		// Disable PawnControlRotation for Camera; Camera should be rotated from Tick function while locked on target
		CameraBoom->bUsePawnControlRotation = false;
	}
	else
	{
		CameraBoom->bUsePawnControlRotation = true;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseDirectionalMovement = false;
	}
}

void ADefianceCharacter::CL_PointCameraToTarget_Implementation(AActor* Target)
{
	if (Target != nullptr)
	{
		// Play CameraRotationTimeline to get Camera on the correct rotation facing the given Target
		CameraStartRotation = Controller->GetControlRotation();
		CameraEndRotation = UKismetMathLibrary::FindLookAtRotation(CameraBoom->GetComponentLocation(), Target->GetActorLocation());
		CameraEndRotation.Pitch += CameraLockPitchAngleOffset;
		PlayCameraRotationTimeline(6.f);
	}
}


ADefianceCharacter* ADefianceCharacter::SelectTargetToLockOn()
{
	//Check if there are any candidate lockable targets
	if (LockableTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ADefianceCharacter [SelectTargetToLockOn]: There are no available targets to lock on to (Empty LockableTargets array). Returning null pointer."))
		return nullptr;
	}

	float FOV = FollowCamera->FieldOfView;
	FVector CameraForwardVector = FollowCamera->GetForwardVector();
	float LastTargetDistanceFromCameraView = TargetLockSphere->GetScaledSphereRadius();
	float TargetDistanceFromCameraView = 0.f; 
	ADefianceCharacter* TargetToLockOn = nullptr;


	for (int8 i = 0; i < LockableTargets.Num(); i++)
	{
		FVector CameraToTargetDirection = LockableTargets[i]->GetActorLocation() - FollowCamera->GetComponentLocation();
		CameraToTargetDirection = CameraToTargetDirection.GetSafeNormal();

		//Check if the target was in players field of view
		if (FMath::RadiansToDegrees(acosf(FVector::DotProduct(CameraForwardVector, CameraToTargetDirection))) < FOV/2)
		{
			FVector TargetLocation = LockableTargets[i]->GetActorLocation();
			FVector CameraLocation = FollowCamera->GetComponentLocation();
			FVector CameraDirection = FollowCamera->GetComponentRotation().Vector();
			TargetDistanceFromCameraView = FMath::PointDistToLine(TargetLocation, CameraDirection, CameraLocation);

			if (TargetDistanceFromCameraView < LastTargetDistanceFromCameraView)
			{
				TargetToLockOn = LockableTargets[i];
				LastTargetDistanceFromCameraView = TargetDistanceFromCameraView;
			}
		}
	}

	return TargetToLockOn;
}


void ADefianceCharacter::CL_ResetCamera_Implementation()
{
	//If previously
	if (bIsLockedOnTarget)
	{
		Controller->SetControlRotation(CameraBoom->GetComponentRotation());
		CameraBoom->bUsePawnControlRotation = true;
	}
	else
	{
		CameraStartRotation = GetControlRotation();
		CameraEndRotation = GetActorRotation();
		PlayCameraRotationTimeline(6.f);
	}
}

void ADefianceCharacter::RotateCamera()
{
	if (LockedOnTarget != nullptr)
	{
		FRotator NewCameraRotation = UKismetMathLibrary::FindLookAtRotation(CameraBoom->GetComponentLocation(), LockedOnTarget->GetActorLocation());
		NewCameraRotation.Pitch += CameraLockPitchAngleOffset;
		CameraBoom->SetWorldRotation(NewCameraRotation);
	}
}

FVector ADefianceCharacter::GetPlayerInputMovementDirectionXY()
{
	// Get the input of the movement action value
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	FInputActionValue MoveInputValue = PlayerInput->GetActionValue(InputConfigData->InputMove);
	FVector2D MovementValue = MoveInputValue.Get<FVector2D>();

	// Get the player camera rotation
	FRotator CameraRotation = FollowCamera->GetComponentRotation();

	return UBasicSupportLibrary::GetPlayerInputMovementDirectionXY(CameraRotation, MovementValue.Y, MovementValue.X);
}

FVector ADefianceCharacter::GetPlayerInputMovementDirection()
{
	// Get the input of the movement action value
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	FInputActionValue MoveInputValue = PlayerInput->GetActionValue(InputConfigData->InputMove);
	FVector2D MovementValue = MoveInputValue.Get<FVector2D>();

	// Get the player camera rotation
	FRotator CameraRotation = FollowCamera->GetComponentRotation();

	return UBasicSupportLibrary::GetPlayerInputMovementDirection(CameraRotation, MovementValue.Y, MovementValue.X);
}



/*------------------TIMELINE FUNCTION OVERLOADS------------------*/
void ADefianceCharacter::OnCameraRotationTimelineUpdate(float Value)
{
	if (!bIsLockedOnTarget)
	{
		Controller->SetControlRotation(FMath::Lerp(CameraStartRotation, CameraEndRotation, Value));
	}
	else
	{
		CameraBoom->SetWorldRotation(FMath::Lerp(CameraStartRotation, CameraEndRotation, Value));
	}
}

void ADefianceCharacter::OnCameraRotationTimelineEnd()
{

}

void ADefianceCharacter::PlayCameraRotationTimeline(float Rate)
{
	CameraRotationTimeline->SetPlayRate(Rate);
	CameraRotationTimeline->PlayFromStart();
}


/*---------------COMPONENT EVENT FUNCTION OVERLOADS---------------*/
void ADefianceCharacter::OnTargetLockSphereBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(HasAuthority())
		NM_AddNewLockableTarget_Implementation(OtherActor);
}

void ADefianceCharacter::NM_AddNewLockableTarget_Implementation(AActor* Actor)
{
	if (Actor == this)
	{
		return;
	}

	LockableTargets.Add((ADefianceCharacter*)Actor);
}

void ADefianceCharacter::OnTargetLockSphereEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == LockedOnTarget)
	{
		// Unlock from target
		ToggleTargetLock();
	}
	
	if (HasAuthority())
		NM_RemoveLockableTarget(OtherActor);
}

void ADefianceCharacter::NM_RemoveLockableTarget_Implementation(AActor* Actor)
{
	if (Actor != nullptr)
		LockableTargets.Remove((ADefianceCharacter*)Actor);
}