// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BaseAnimInst.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UBaseAnimInst::NativeInitializeAnimation()
{
	// Getting the references
	OwnerCharacter = Cast<ACharacter>(GetOwningActor());

	if (OwnerCharacter != nullptr)
		MovementComp = OwnerCharacter->GetCharacterMovement();
}


void UBaseAnimInst::NativeUpdateAnimation(float DeltaTimeX)
{


}





float UBaseAnimInst::CalculateGroundSpeedBase3()
{
	return (GroundSpeed / MovementComp->MaxWalkSpeed) * 3;
}

float UBaseAnimInst::CalculateStride()
{
	if (GroundSpeed < MaxStrideSpeed)
	{
		return GroundSpeed / MaxStrideSpeed;
	}
	else
	{
		return 1.f;
	}
}

FVelocityBlend UBaseAnimInst::CalculateVelocityBlend()
{
	FVelocityBlend NewVelocityBlend;

	//Calculate Normalized Velocity
	FVector NormalizedVelocity = Velocity;
	NormalizedVelocity.Normalize();

	//Get location relative velocity direction
	FVector LocRelativeVelocityDir = OwnerCharacter->GetActorRotation().UnrotateVector(NormalizedVelocity);
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

FVelocityBlend UBaseAnimInst::InterpVelocityBlend(FVelocityBlend CurrentVB, FVelocityBlend TargetVB, float InterpSpeed, float DeltaTime)
{
	FVelocityBlend InterpolatedVB;

	//Interpolate between current and target velocity blends
	InterpolatedVB.Forward = FMath::FInterpTo(CurrentVB.Forward, TargetVB.Forward, InterpSpeed, DeltaTime);
	InterpolatedVB.Backward = FMath::FInterpTo(CurrentVB.Backward, TargetVB.Backward, InterpSpeed, DeltaTime);
	InterpolatedVB.Left = FMath::FInterpTo(CurrentVB.Left, TargetVB.Left, InterpSpeed, DeltaTime);
	InterpolatedVB.Right = FMath::FInterpTo(CurrentVB.Right, TargetVB.Right, InterpSpeed, DeltaTime);

	return InterpolatedVB;
}

EMovementDirection UBaseAnimInst::CalculateMovementDirection(float FR_Threshold, float FL_Threshold, float BR_Threshold, float BL_Threshold, float Buffer)
{
	FRotator DeltaRotation = Velocity.Rotation() - OwnerCharacter->GetActorRotation();

	bool bIncreaseBuffer = (MovementDirection != EMovementDirection::MD_F);
	bool AngleBetweenFL_FR = AngleInRange(DeltaRotation.Yaw, FL_Threshold, FR_Threshold, Buffer, bIncreaseBuffer);
	bIncreaseBuffer = (MovementDirection != EMovementDirection::MD_R);
	bool AngleBetweenFR_BR = AngleInRange(DeltaRotation.Yaw, FR_Threshold, BR_Threshold, Buffer, bIncreaseBuffer);
	bIncreaseBuffer = (MovementDirection != EMovementDirection::MD_L);
	bool AngleBetweenBL_FL = AngleInRange(DeltaRotation.Yaw, BL_Threshold, FL_Threshold, Buffer, bIncreaseBuffer);

	if (AngleBetweenFL_FR)
	{
		return EMovementDirection::MD_F;
	}
	else if (AngleBetweenFR_BR)
	{
		return EMovementDirection::MD_R;
	}
	else if (AngleBetweenBL_FL)
	{
		return EMovementDirection::MD_L;
	}
	else
	{
		return EMovementDirection::MD_B;
	}
}

bool UBaseAnimInst::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool bIncreaseBuffer)
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

FVector2D UBaseAnimInst::CalculateLeanFactor()
{
	////check if character is NULL of if character is in the air
	//if ((OwnerCharacter == nullptr) || (bIsFalling))
	//	return FVector2D(0.f, 0.f);

	//if (!bUseDirectionalMovement)
	//{
	//	FVector NormalizedVelocity = Velocity;
	//	NormalizedVelocity.Normalize();
	//	float LeanRight = FVector::DotProduct(TryGetPawnOwner()->GetActorRightVector(), NormalizedVelocity);
	//	return FVector2D(-LeanRight * GroundSpeed / OwnerCharacter->MaxRunSpeed, 0.f);

	//}
	//else
	//{
	//	FVector NormalizedVelocity = Velocity;
	//	NormalizedVelocity.Normalize();
	//	FVector RelativeNormVelocity = OwnerCharacter->GetMesh()->GetComponentRotation().UnrotateVector(NormalizedVelocity);

	//	return FVector2D(-RelativeNormVelocity.X * GroundSpeed / OwnerCharacter->MaxSprintSpeed, RelativeNormVelocity.Y * GroundSpeed / Character->MaxSprintSpeed);
	//}

	return FVector2D(0.f, 0.f);
}

void UBaseAnimInst::PlayDirectionalMontage(float Angle, TMap<EDetailedDirection, UAnimMontage*> MontageMap)
{
	//Forward
	if (Angle <= 22.5f && Angle > -22.5f) {
		if (MontageMap.Contains(EDetailedDirection::Forward)) {
			Montage_Play(MontageMap[EDetailedDirection::Forward]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::Forward]->GetFName().ToString(), Angle)
		}
	}
	//Forward Right
	else if (Angle >= 22.5f && Angle < 67.5f) {
		if (MontageMap.Contains(EDetailedDirection::ForwardRight)) {
			Montage_Play(MontageMap[EDetailedDirection::ForwardRight]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::ForwardRight]->GetFName().ToString(), Angle)
		}
	}
	//Right
	else if (Angle >= 67.5f && Angle < 112.5) {
		if (MontageMap.Contains(EDetailedDirection::Right)) {
			Montage_Play(MontageMap[EDetailedDirection::Right]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::Right]->GetFName().ToString(), Angle)
		}
	}
	//Backward Right
	else if (Angle >= 112.5f && Angle < 157.5f) {
		if (MontageMap.Contains(EDetailedDirection::BackwardRight)) {
			Montage_Play(MontageMap[EDetailedDirection::BackwardRight]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::BackwardRight]->GetFName().ToString(), Angle)
		}
	}
	//Backward
	else if (Angle <= -157.5f || Angle >= 157.5f) {
		if (MontageMap.Contains(EDetailedDirection::Backward)) {
			Montage_Play(MontageMap[EDetailedDirection::Backward]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::Backward]->GetFName().ToString(), Angle)
		}
	}
	//Backward Left
	else if (Angle <= -112.5f && Angle > -157.5f) {
		if (MontageMap.Contains(EDetailedDirection::BackwardLeft)) {
			Montage_Play(MontageMap[EDetailedDirection::BackwardLeft]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::BackwardLeft]->GetFName().ToString(), Angle)
		}
	}
	//Left
	else if (Angle <= -67.5f && Angle > -122.5f) {
		if (MontageMap.Contains(EDetailedDirection::Left)) {
			Montage_Play(MontageMap[EDetailedDirection::Left]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::Left]->GetFName().ToString(), Angle)
		}
	}
	//Forward Left
	else if (Angle <= -22.5f && Angle > -67.5f) {
		if (MontageMap.Contains(EDetailedDirection::ForwardLeft)) {
			Montage_Play(MontageMap[EDetailedDirection::ForwardLeft]);
			UE_LOG(LogTemp, Log, TEXT("UBaseAnimInstance_ABP [PlayDirectionalMontage]: Playing %s at Angle %f."), *MontageMap[EDetailedDirection::ForwardLeft]->GetFName().ToString(), Angle)
		}
	}
}


