// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Types.h"
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInst.generated.h"

/**
 * 
 */
UCLASS()
class DEFIANCE_API UBaseAnimInst : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Called at start of play */
	virtual void NativeInitializeAnimation() override;

	/** Called every frame */
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
	class ACharacter* OwnerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
	class UCharacterMovementComponent* MovementComp;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Essential Movement Data")
	FVector Velocity;

	/** Indicates movement speed of the Pawn */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float GroundSpeed;

	/** Indicates movement speed of the Pawn normalized in the range 0-3 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float GroundSpeedBase3;




	/** Indicates the stride the pawn should take when moving; Range 0-1 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float Stride;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float MaxStrideSpeed = 200.f;

	/** Indicates whether the Pawn should move or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	bool bShouldMove;

	/** Indicates whether the Pawn is Crouching, Walking, Running or Sprinting */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	EMovementStance MovementStance;

	/** Indicates whether the Pawn is falling or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	bool bIsFalling;

	/** Indicates whether the Pawn is crouching or not */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	bool bIsCrouching;

	/** Velocity broken down to 4 vectors on the XY axis (+X, -X, +Y, -Y) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	FVelocityBlend VelocityBlend;

	/** Indicates the quadrant of the movement direction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	EMovementDirection MovementDirection;

	/** Indicates whether the character should use directional movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	bool bUseDirectionalMovement = false;

	/** Indicates the lean factor when pawn is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Essential Movement Data")
	FVector2D LeanFactor;

	/** A collection of dodge animation montages mapped according to direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge & Roll|Dodge")
	TMap<EDetailedDirection, UAnimMontage*> DodgeAnimMontage;

	/** A collection of roll animation montages mapped according to direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge & Roll|Roll")
	TMap<EDetailedDirection, UAnimMontage*> RollAnimMontage;







	/** Calculates the gait based on the currect speed and the maximum speed */
	UFUNCTION(BlueprintCallable)
	float CalculateGroundSpeedBase3();

	/** Calculates the stride value the pawn should take */
	UFUNCTION(BlueprintCallable)
	float CalculateStride();

	/** Calculates the velocity blend */
	UFUNCTION(BlueprintCallable)
	FVelocityBlend CalculateVelocityBlend();

	/** Returns the interpolation between the current velocity blendand the target velocity blend */
	UFUNCTION(BlueprintCallable)
	FVelocityBlend InterpVelocityBlend(FVelocityBlend CurrentVB, FVelocityBlend TargetVB, float InterpSpeed, float DeltaTime);

	/** Calculates the movement direction according to the specified thresholds */
	UFUNCTION(BlueprintCallable)
	EMovementDirection CalculateMovementDirection(float FR_Threshold, float FL_Threshold, float BR_Threshold, float BL_Threshold, float Buffer);

	/** Returns true if the angle is within the specified range with the buffer tolerance */
	UFUNCTION(BlueprintCallable)
	bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool bIncreaseBuffer);

	/** Calculate the lean factor (-1; 1) for the pawn when moving */
	UFUNCTION(BlueprintCallable)
	FVector2D CalculateLeanFactor();

	/* Plays the directional montage from the given selection according to the given Angle */
	UFUNCTION(BlueprintCallable)
	void PlayDirectionalMontage(float Angle, TMap<EDetailedDirection, UAnimMontage*> MontageMap);

	
};
