// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Types.h"
#include "BaseAnimInstance_ABP.generated.h"

/**
 * 
 */
UCLASS()
class DEFIANCE_API UBaseAnimInstance_ABP : public UAnimInstance
{
	GENERATED_BODY()


public:
	/** Called at start of play */
	virtual void NativeInitializeAnimation() override;

	/** Called every frame */
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;



	/** The owning ADefianceCharacter of this Anim BP */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	class ADefianceCharacter* Character;

	/** The character movement component of the owning ADefianceCharacter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	class UCharacterMovementComponent* CharacterMovement;


	/** Indicates whether the Pawn is Crouching, Walking, Running or Sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	EMovementStance MovementStance = EMovementStance::Running;

	UFUNCTION(BlueprintCallable)
	void HandleUpdatedMovementStance(EMovementStance NewMovementStance);

	/** Indicates whether the character should use directional movement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	bool bUseDirectionalMovement = false;

	UFUNCTION(BlueprintCallable)
	void HandleUpdatedLockOn(AActor* NewTargetActorRef);




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Movement Data")
	FVector Velocity;

	/** Indicates movement speed of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float GroundSpeed;

	/** Indicates movement speed of the Pawn normalized in the range 0-3 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float GroundSpeedBase3;

	/** Indicates the stride the pawn should take when moving; Range 0-1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float Stride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	float MaxStrideSpeed = 200.f;

	/** Indicates whether the Pawn should move or not */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	bool bShouldMove;

	/** Indicates whether the Pawn is falling or not */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	bool bIsFalling;

	UFUNCTION(BlueprintCallable)
	void UpdateMovementState();


	/** Indicates the quadrant of the movement direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	EMovementDirection MovementDirection;

	UFUNCTION(BlueprintCallable)
	void UpdateMovementDirection(float FR_Threshold, float FL_Threshold, float BR_Threshold, float BL_Threshold, float Buffer);


	/** Velocity broken down to 4 vectors on the XY axis (+X, -X, +Y, -Y) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Essential Movement Data")
	FVelocityBlend VelocityBlend;

	UFUNCTION(BlueprintCallable)
	void UpdateVelocityBlend(float DeltaTimeX);

	/** Calculates the velocity blend */
	FVelocityBlend CalculateVelocityBlend();

	/** Returns the interpolation between the current velocity blendand the target velocity blend */
	FVelocityBlend InterpVelocityBlend(FVelocityBlend CurrentVB, FVelocityBlend TargetVB, float InterpSpeed, float DeltaTime);


	/** Indicates the lean factor when pawn is moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Movement Data")
	FVector2D LeanFactor;

	UPROPERTY(EditAnywhere, Category = "Essential Movement Data")
	float LeanMultiplier{ 3.0f };

	/** Calculate the lean factor (-1; 1) for the pawn when moving */
	UFUNCTION(BlueprintCallable)
	void UpdateLeanFactor();


	/** Returns true if the angle is within the specified range with the buffer tolerance */
	bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool bIncreaseBuffer);

	


	

	


};
