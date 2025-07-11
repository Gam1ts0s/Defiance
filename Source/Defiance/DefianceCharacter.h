// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types.h"
#include "Curves/CurveFloat.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h"
#include "Interfaces/Enemy.h"
#include "DefianceCharacter.generated.h"

UCLASS(config=Game)
class ADefianceCharacter : public ACharacter, public IEnemy
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	ADefianceCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ULockOnComponent* LockOnComponent;


	/** Keep this as clean as possible */
	virtual void Tick(float DeltaTime);


protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;



	/** Animation instance of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = "true"))
	class UBaseAnimInstance_ABP* AnimInstance;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }



	/** Capsule Radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical Properties|Parameters")
	float CapsuleRadius = 35.f;

	/** Capsule Half Height when the Pawn is standing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical Properties|Parameters")
	float CapsuleHalfHeight = 90.f;

	/** Capsule Half Height when the Pawn is crouching */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical Properties|Parameters")
	float CapsuleHalfHeightCrouched = 60.f;

};


