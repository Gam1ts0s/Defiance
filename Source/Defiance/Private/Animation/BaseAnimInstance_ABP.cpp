// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BaseAnimInstance_ABP.h"
#include "Kismet/KismetMathLibrary.h"
#include "../DefianceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBaseAnimInstance_ABP::NativeInitializeAnimation()
{
	Character = Cast<ADefianceCharacter>(GetOwningActor());
	if (IsValid(Character))
	{
		CharacterMovement = Character->GetCharacterMovement();
	}
}

void UBaseAnimInstance_ABP::NativeUpdateAnimation(float DeltaTimeX)
{
	
}


void UBaseAnimInstance_ABP::UpdateMovementState()
{	
	if (!IsValid(Character) || !IsValid(CharacterMovement)) { return; }

	Velocity = CharacterMovement->Velocity;

	GroundSpeed = UKismetMathLibrary::VSizeXY(Velocity);
	GroundSpeedBase3 = (GroundSpeed / CharacterMovement->MaxWalkSpeed) * 3;

	bShouldMove = (GroundSpeed > 3) && (!CharacterMovement->GetCurrentAcceleration().IsZero());

	bIsFalling = CharacterMovement->IsFalling();

	Stride = (GroundSpeed < MaxStrideSpeed) ? GroundSpeed / MaxStrideSpeed : 1.0f;
}



void UBaseAnimInstance_ABP::UpdateVelocityBlend(float DeltaTimeX)
{
	if (!IsValid(Character)) { return; }

	FVelocityBlend TargetVelocityBlend = CalculateVelocityBlend();
	VelocityBlend = InterpVelocityBlend(VelocityBlend, TargetVelocityBlend, 12.f, DeltaTimeX);
}

FVelocityBlend UBaseAnimInstance_ABP::CalculateVelocityBlend()
{
	FVelocityBlend NewVelocityBlend;

	//Calculate Normalized Velocity
	FVector NormalizedVelocity = Velocity;
	NormalizedVelocity.Normalize();

	//Get location relative velocity direction
	FVector LocRelativeVelocityDir = Character->GetActorRotation().UnrotateVector(NormalizedVelocity);
	float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) + FMath::Abs(LocRelativeVelocityDir.Z);

	//Get relative direction
	FVector RelativeDirection = LocRelativeVelocityDir / Sum;

	//Get velocity blends by clamping values
	NewVelocityBlend.Forward = FMath::Clamp(RelativeDirection.X, 0.f, 1.f);
	NewVelocityBlend.Backward = FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.f, 0.f));
	NewVelocityBlend.Left = FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.f, 0.f));
	NewVelocityBlend.Right = FMath::Clamp(RelativeDirection.Y, 0.f, 1.f);

	return NewVelocityBlend;
}


FVelocityBlend UBaseAnimInstance_ABP::InterpVelocityBlend(FVelocityBlend CurrentVB, FVelocityBlend TargetVB, float InterpSpeed, float DeltaTime)
{
	FVelocityBlend InterpolatedVB;

	//Interpolate between current and target velocity blends
	InterpolatedVB.Forward = FMath::FInterpTo(CurrentVB.Forward, TargetVB.Forward, InterpSpeed, DeltaTime);
	InterpolatedVB.Backward = FMath::FInterpTo(CurrentVB.Backward, TargetVB.Backward, InterpSpeed, DeltaTime);
	InterpolatedVB.Left = FMath::FInterpTo(CurrentVB.Left, TargetVB.Left, InterpSpeed, DeltaTime);
	InterpolatedVB.Right = FMath::FInterpTo(CurrentVB.Right, TargetVB.Right, InterpSpeed, DeltaTime);

	return InterpolatedVB;
}


void UBaseAnimInstance_ABP::UpdateMovementDirection(float FR_Threshold, float FL_Threshold, float BR_Threshold, float BL_Threshold, float Buffer)
{
	if (!IsValid(Character)) { return; }

	FRotator DeltaRotation = Velocity.Rotation() - Character->GetActorRotation();

	bool bIncreaseBuffer = (MovementDirection != EMovementDirection::MD_F);
	bool AngleBetweenFL_FR = AngleInRange(DeltaRotation.Yaw, FL_Threshold, FR_Threshold, Buffer, bIncreaseBuffer);
	bIncreaseBuffer = (MovementDirection != EMovementDirection::MD_R);
	bool AngleBetweenFR_BR = AngleInRange(DeltaRotation.Yaw, FR_Threshold, BR_Threshold, Buffer, bIncreaseBuffer);
	bIncreaseBuffer = (MovementDirection != EMovementDirection::MD_L);
	bool AngleBetweenBL_FL = AngleInRange(DeltaRotation.Yaw, BL_Threshold, FL_Threshold, Buffer, bIncreaseBuffer);

	if (AngleBetweenFL_FR)
	{
		MovementDirection = EMovementDirection::MD_F;
	}
	else if (AngleBetweenFR_BR)
	{
		MovementDirection = EMovementDirection::MD_R;
	}
	else if (AngleBetweenBL_FL)
	{
		MovementDirection = EMovementDirection::MD_L;
	}
	else
	{
		MovementDirection = EMovementDirection::MD_B;
	}
}




bool UBaseAnimInstance_ABP::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool bIncreaseBuffer)
{
	if (bIncreaseBuffer)
	{
		return UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle - Buffer, MaxAngle + Buffer);
	}
	else
	{
		return UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle + Buffer, MaxAngle - Buffer);
	}
}




void UBaseAnimInstance_ABP::UpdateLeanFactor()
{
	if ((!IsValid(Character)) || (bIsFalling))
	{
		LeanFactor = FVector2D(0.f, 0.f);
		return;
	}

	if (!bUseDirectionalMovement)
	{
		FVector NormalizedVelocity = Velocity;
		NormalizedVelocity.Normalize();
		float LeanRight = FVector::DotProduct(TryGetPawnOwner()->GetActorRightVector(), NormalizedVelocity);
		LeanFactor = FVector2D(-LeanRight * LeanMultiplier * GroundSpeed / CharacterMovement->MaxWalkSpeed, 0.f);
	}
	else
	{
		FVector NormalizedVelocity = Velocity;
		NormalizedVelocity.Normalize();
		FVector RelativeNormVelocity = Character->GetMesh()->GetComponentRotation().UnrotateVector(NormalizedVelocity);
		LeanFactor = FVector2D(-RelativeNormVelocity.X * GroundSpeed / CharacterMovement->MaxWalkSpeed, RelativeNormVelocity.Y * GroundSpeed / CharacterMovement->MaxWalkSpeed);
	}
}




void UBaseAnimInstance_ABP::HandleUpdatedLockOn(AActor* NewTargetActorRef)
{
	bUseDirectionalMovement = IsValid(NewTargetActorRef);
}

void UBaseAnimInstance_ABP::HandleUpdatedMovementStance(EMovementStance NewMovementStance)
{
	MovementStance = NewMovementStance;
}

