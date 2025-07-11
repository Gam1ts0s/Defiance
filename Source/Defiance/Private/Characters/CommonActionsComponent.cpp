// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CommonActionsComponent.h"
#include "BasicSupportLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"



// Sets default values for this component's properties
UCommonActionsComponent::UCommonActionsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}

void UCommonActionsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCommonActionsComponent, bUseDirectionalMovement);
	DOREPLIFETIME(UCommonActionsComponent, MovementStance);
	DOREPLIFETIME(UCommonActionsComponent, bCanJump);
	DOREPLIFETIME(UCommonActionsComponent, bCanSprint);
	DOREPLIFETIME(UCommonActionsComponent, bIsSprinting);
	DOREPLIFETIME(UCommonActionsComponent, bCanCrouch);
	DOREPLIFETIME(UCommonActionsComponent, bIsCrouching);
	DOREPLIFETIME(UCommonActionsComponent, bCanDodge);
	DOREPLIFETIME(UCommonActionsComponent, bIsDodging);
	DOREPLIFETIME(UCommonActionsComponent, bCanRoll);
	DOREPLIFETIME(UCommonActionsComponent, bIsRolling);
}

void UCommonActionsComponent::HandleUpdatedUseDirectionalMovement(bool bNewUseDirectionalMovement)
{
	bUseDirectionalMovement = bNewUseDirectionalMovement;
}




// Called when the game starts
void UCommonActionsComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerRef = GetOwner<ACharacter>();
	Controller = GetWorld()->GetFirstPlayerController();
	MovementComp = OwnerRef->GetCharacterMovement();
	CameraBoomLocation = OwnerRef->FindComponentByClass<USpringArmComponent>()->GetRelativeLocation();


	// Initialization
	MovementComp->MaxWalkSpeed = MaxRunSpeed;
	MovementComp->MaxWalkSpeedCrouched = MaxCrouchSpeed;
	MovementComp->NavAgentProps.bCanCrouch = bCanCrouch;	
}


// Called every frame
void UCommonActionsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UCommonActionsComponent::OnRep_MovementStance()
{
	OnUpdatedMovementStanceDelegate.Broadcast(MovementStance);
}



void UCommonActionsComponent::Jump()
{
	if (!IsValid(MovementComp) || !IsValid(OwnerRef)) { return; }
	if (!bCanJump) { return; }

	if (bIsCrouching)
	{
		SR_EndCrouch();
	}
	else
	{
		if (OwnerRef->CanJump()) { OwnerRef->Jump(); }
	}
}

void UCommonActionsComponent::StopJumping()
{
	if (!IsValid(MovementComp) || !IsValid(OwnerRef)) { return; }

	OwnerRef->StopJumping();
}




void UCommonActionsComponent::ToggleSprint()
{
	if (!IsValid(MovementComp) || !IsValid(OwnerRef)) { return; }

	if (!bIsSprinting)
	{
		SR_StartSprint();
	}
	else
	{
		SR_EndSprint();
	}
}

void UCommonActionsComponent::SR_StartSprint_Implementation()
{
	if (MovementComp->IsFalling()) { return; }
	if (!bCanSprint) { return; }

	bIsCrouching = false;
	OnRep_IsCrouching();
	bIsSprinting = true;
	OnRep_IsSprinting();
	MovementStance = EMovementStance::Sprinting;
	OnRep_MovementStance();
}

void UCommonActionsComponent::SR_EndSprint_Implementation()
{
	bIsSprinting = false;
	OnRep_IsSprinting();
	MovementStance = EMovementStance::Running;
	OnRep_MovementStance();
}

void UCommonActionsComponent::OnRep_IsSprinting()
{
	if (bIsSprinting) {
		MovementComp->MaxWalkSpeed = MaxSprintSpeed;

		//If pawn currently uses directional movement change to non-directional movement
		if (bUseDirectionalMovement)
		{
			MovementComp->bUseControllerDesiredRotation = false;
			MovementComp->bOrientRotationToMovement = true;
		}
	}
	else
	{
		MovementComp->MaxWalkSpeed = MaxRunSpeed;

		//If character is locked on target when sprint ends then change to directional movement
		if (bUseDirectionalMovement)
		{
			MovementComp->bUseControllerDesiredRotation = true;
			MovementComp->bOrientRotationToMovement = false;
		}
	}
}







void UCommonActionsComponent::ToggleCrouch()
{
	if (!IsValid(MovementComp) || !IsValid(OwnerRef)) { return; }

	if (!bIsCrouching)
	{
		SR_StartCrouch();
	}
	else
	{
		SR_EndCrouch();
	}
}


void UCommonActionsComponent::OnRep_IsCrouching()
{
	if (OwnerRef->IsLocallyControlled())
	{
		if (bIsCrouching)
		{
			OwnerRef->Crouch();

			USpringArmComponent* CameraBoom{ OwnerRef->FindComponentByClass<USpringArmComponent>() };
			float CapsuleHalfHeight{ OwnerRef->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() };
			float CapsuleHalfHeightCrouched{ MovementComp->GetCrouchedHalfHeight() };
			CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, (CapsuleHalfHeight - CapsuleHalfHeightCrouched)));
		}
		else
		{
			OwnerRef->UnCrouch();

			USpringArmComponent* CameraBoom{ OwnerRef->FindComponentByClass<USpringArmComponent>() };
			CameraBoom->SetRelativeLocation(CameraBoomLocation);
		}
	}
}


void UCommonActionsComponent::SR_StartCrouch_Implementation()
{
	if (MovementComp->IsFalling()) { return; }
	if (!bCanCrouch || bIsSprinting || bIsDodging || bIsRolling) { return; }
	if (!OwnerRef->CanCrouch()) { return; }

	bIsCrouching = true;
	OnRep_IsCrouching();
	MovementStance = EMovementStance::Crouched;
	OnRep_MovementStance();

}

void UCommonActionsComponent::SR_EndCrouch_Implementation()
{
	bIsCrouching = false;
	OnRep_IsCrouching();
	MovementStance = EMovementStance::Running;
	OnRep_MovementStance();
}






void UCommonActionsComponent::DodgeRoll()
{
	if (!IsValid(MovementComp) || !IsValid(OwnerRef)) { return; }
	if (MovementComp->IsFalling()) { return; }
	if (!bCanDodge && !bCanRoll) { return; }
	
	// Determine the direction the player wishes to Dodge/Roll. 
	FVector MovementDirection{ (MovementComp->Velocity.Length() < 1) ? OwnerRef->GetActorForwardVector() : MovementComp->GetLastInputVector() };
	FRotator MovementRotationAroundPlayer = OwnerRef->GetTransform().InverseTransformVector(MovementDirection).Rotation();
	float Angle{ static_cast<float>(MovementRotationAroundPlayer.Yaw)};
	EDetailedDirection DetailedDirection{ UBasicSupportLibrary::GetDetailedDirectionFromAngle(Angle) };

	if (bCanDodge && !bIsDodging && !bIsRolling)
	{
		SR_Dodge(DetailedDirection);
	}
	else if (bCanRoll && !bIsRolling)
	{
		SR_Roll(DetailedDirection);
	}

}



bool UCommonActionsComponent::SR_Dodge_Validate(EDetailedDirection DetailedDirection)
{
	if (!bCanDodge || bIsDodging) { return false; }
	return true;
}

void UCommonActionsComponent::SR_Dodge_Implementation(EDetailedDirection DetailedDirection)
{
	if (!DodgeAnimMontage.Contains(DetailedDirection)) return;

	bIsCrouching = false;
	OnRep_IsCrouching();
	bIsDodging = true;
	NM_PlayDodgeAnim(DetailedDirection);
}

void UCommonActionsComponent::NM_PlayDodgeAnim_Implementation(EDetailedDirection DetailedDirection)
{
	float Duration{ OwnerRef->PlayAnimMontage(DodgeAnimMontage[DetailedDirection]) };

	FTimerHandle DodgeTimerHandle;
	OwnerRef->GetWorldTimerManager().SetTimer(
		DodgeTimerHandle,
		this,
		&UCommonActionsComponent::FinishDodgeAnim,
		Duration,
		false
	);
}

void UCommonActionsComponent::FinishDodgeAnim()
{
	bIsDodging = false;
}



bool UCommonActionsComponent::SR_Roll_Validate(EDetailedDirection DetailedDirection)
{
	if (!bCanRoll || bIsRolling) { return false; }
	return true;
}

void UCommonActionsComponent::SR_Roll_Implementation(EDetailedDirection DetailedDirection)
{
	if (!RollAnimMontage.Contains(DetailedDirection)) return;

	bIsCrouching = false;
	OnRep_IsCrouching();
	bIsRolling = true;
	NM_PlayRollAnim(DetailedDirection);
}

void UCommonActionsComponent::NM_PlayRollAnim_Implementation(EDetailedDirection DetailedDirection)
{
	float Duration{ OwnerRef->PlayAnimMontage(RollAnimMontage[DetailedDirection]) };

	FTimerHandle RollTimerHandle;
	OwnerRef->GetWorldTimerManager().SetTimer(
		RollTimerHandle,
		this,
		&UCommonActionsComponent::FinishRollAnim,
		Duration,
		false
	);
}

void UCommonActionsComponent::FinishRollAnim()
{
	bIsDodging = false;
	bIsRolling = false;
}