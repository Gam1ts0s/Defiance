// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/LockOnComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interfaces/Enemy.h"


// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);

}


// Called when the game starts
void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerRef = GetOwner<ACharacter>();
	Controller = GetWorld()->GetFirstPlayerController();
	MovementComp = OwnerRef->GetCharacterMovement();
	CameraBoom = OwnerRef->FindComponentByClass<USpringArmComponent>();

	
}

void ULockOnComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate Target Lock Variables
	DOREPLIFETIME(ULockOnComponent, CurrentTargetActor);
}



// Called every frame
void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//bool OwnedLocally = OwnerRef->IsOwnedBy(UGameplayStatics::GetPlayerController(this, 0));
	bool LocallyControlled{ OwnerRef->IsLocallyControlled() };
	if (LocallyControlled) {
		FVector CurrentLocation;
		FVector TargetLocation;
		FRotator LockOnRotation;

		// Check if there is currently a locked on target
		if (!IsValid(CurrentTargetActor)) { return; }

		// Calculate distance between owner and target. If greater than BreakDistance end the lock on
		TargetLocation = CurrentTargetActor->GetActorLocation();
		CurrentLocation = OwnerRef->GetActorLocation();
		double TargetDistance{ FVector::Distance(CurrentLocation, TargetLocation) };


		if (TargetDistance >= BreakDistance)
		{
			EndLockOn();
			return;
		}


		// Change Controller rotation
		LockOnRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
		LockOnRotation.Pitch += CameraPitchCorrection;
		Controller->SetControlRotation(LockOnRotation);
	}
}


void ULockOnComponent::ResetCamera()
{	
	if (IsValid(Controller))
	{
		FVector PawnForwardVector{ Controller->GetPawn()->GetActorForwardVector() };
		FRotator PawnForwardRotation{ PawnForwardVector.Rotation()};
		PawnForwardRotation.Pitch += CameraPitchCorrection;
		Controller->SetControlRotation(PawnForwardRotation);
	}
}



void ULockOnComponent::ToggleLockOn(float SphereRadious)
{
	//SR_ToggleLockOn(SphereRadious);
	if (IsValid(CurrentTargetActor))
	{
		EndLockOn();
	}
	else
	{
		if (!StartLockOn(SphereRadious))
		{
			ResetCamera();
		}
	}
}



bool ULockOnComponent::StartLockOn(float SphereRadious)
{
	// First detect valid targets within SphereRadious
	//FHitResult OutResult;
	FVector CurrentLocation{ OwnerRef->GetActorLocation() };
	FCollisionShape Sphere{ FCollisionShape::MakeSphere(SphereRadious) };
	FCollisionQueryParams IgnoreParams{
		FName{TEXT("Ignore Collision Params")},
		false,
		OwnerRef
	};
	TArray<FHitResult> OutHit;

	bool bHasFoundTarget{ GetWorld()->SweepMultiByChannel(
		OutHit,
		CurrentLocation,
		CurrentLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel1,
		Sphere,
		IgnoreParams
	) };

	//UE_LOG(LogTemp, Warning, TEXT("LockOnComponent [StartLockOn]: Detected %d valid targets to lock on."), OutHit.Num())

	if (!bHasFoundTarget) { return 0; }

	// Among all valid targets find the best to lock onto	
	UCameraComponent* OwnerCamera{ OwnerRef->FindComponentByClass<UCameraComponent>() };
	float FOV{ OwnerCamera->FieldOfView };
	FVector CameraForwardVector{ OwnerCamera->GetForwardVector() };
	float LastTargetDistanceFromCameraView{ SphereRadious };
	float TargetDistanceFromCameraView{ 0.f };
	AActor* NewTarget{ nullptr };

	for (int8 i = 0; i < OutHit.Num(); i++)
	{
		AActor* CanditateTarget{ OutHit[i].GetActor() };
		FVector CameraToTargetDirection{ CanditateTarget->GetActorLocation() - OwnerCamera->GetComponentLocation() };
		CameraToTargetDirection = CameraToTargetDirection.GetSafeNormal();

		//Check if the candidate was in players field of view
		if (FMath::RadiansToDegrees(acosf(FVector::DotProduct(CameraForwardVector, CameraToTargetDirection))) < FOV / 2)
		{
			FVector TargetLocation = CanditateTarget->GetActorLocation();
			FVector CameraLocation = OwnerCamera->GetComponentLocation();
			FVector CameraDirection = OwnerCamera->GetComponentRotation().Vector();
			TargetDistanceFromCameraView = FMath::PointDistToLine(TargetLocation, CameraDirection, CameraLocation);

			if (TargetDistanceFromCameraView < LastTargetDistanceFromCameraView)
			{
				NewTarget = CanditateTarget;
				LastTargetDistanceFromCameraView = TargetDistanceFromCameraView;
			}
		}
	}
	
	// Check if the NewTarget is a valid target
	if (!IsValid(NewTarget)) { return 0; }
	if (!NewTarget->Implements<UEnemy>()) { return 0; }
	
	//UE_LOG(LogTemp, Warning, TEXT("LockOnComponent [StartLockOn]: Best candidate to lock on is %s."), *NewTarget->GetName())

	// Perform all LockOn operations
	Controller->SetIgnoreLookInput(true);
	SR_UpdateLockOn(NewTarget);
	
	bool LocallyControlled{ OwnerRef->IsLocallyControlled() };
	if (LocallyControlled)
	{
		IEnemy::Execute_OnSelect(NewTarget);
	}

	return 1;
}

void ULockOnComponent::EndLockOn()
{
	bool LocallyControlled{ OwnerRef->IsLocallyControlled() };
	if (LocallyControlled)
	{
		if (IsValid(CurrentTargetActor))
		{
			IEnemy::Execute_OnDeselect(CurrentTargetActor);
		}
	}
	Controller->ResetIgnoreLookInput();
	SR_UpdateLockOn(nullptr);
}


void ULockOnComponent::OnRep_CurrentTargetActor()
{
	if (OwnerRef->HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
			FString::Printf(TEXT("LockOnComponent [UpdateLockOn]: Server - updating LockOn Target")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow,
			FString::Printf(TEXT("LockOnComponent [UpdateLockOn]: Client %d - updating LockOn Target"), GPlayInEditorID));
	}
	


	if (IsValid(CurrentTargetActor))
	{
		MovementComp->bOrientRotationToMovement = false;
		MovementComp->bUseControllerDesiredRotation = true;

		OnUpdatedTargetDelegate.Broadcast(CurrentTargetActor);
	}
	else
	{
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->bUseControllerDesiredRotation = false;

		OnUpdatedTargetDelegate.Broadcast(CurrentTargetActor);
	}
}


void ULockOnComponent::SR_UpdateLockOn_Implementation(AActor* NewTarget)
{
	CurrentTargetActor = NewTarget;
	OnRep_CurrentTargetActor();
}


bool ULockOnComponent::SR_UpdateLockOn_Validate(AActor* NewTarget)
{
	if (IsValid(NewTarget)) {
		FVector CurrentLocation{ OwnerRef->GetActorLocation() };
		FVector TargetLocation{ NewTarget->GetActorLocation() };
		double TargetDistance{ FVector::Distance(CurrentLocation, TargetLocation) };

		if (TargetDistance <= (BreakDistance + 200.0f))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}

