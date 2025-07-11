// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types.h"
#include "Interfaces/Enemy.h"
#include "BaseCharacter.generated.h"

UCLASS()
class DEFIANCE_API ABaseCharacter : public ACharacter, public IEnemy
{
	GENERATED_BODY()


	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	// Property replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UFUNCTION(BlueprintCallable)
	void HandleUpdatedLockOn(AActor* NewTargetActorRef);

	/** Capsule Radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical Properties|Parameters")
	float CapsuleRadius = 35.f;

	/** Capsule Half Height when the Pawn is standing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical Properties|Parameters")
	float CapsuleHalfHeight = 90.f;

	/** Capsule Half Height when the Pawn is crouching */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physical Properties|Parameters")
	float CapsuleHalfHeightCrouched = 60.f;

	/** The maximum speed the pawn can move when crouched */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Parameters")
	float MaxCrouchSpeed = 200.f;

	/** The maximum speed the pawn can move when walking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Parameters")
	float MaxRunSpeed = 500.f;

	/** The maximum speed the pawn can move when walking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Parameters")
	float MaxSprintSpeed = 1000.f;

	/** Indicates whether the character should use directional movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement|Essential Variables")
	bool bUseDirectionalMovement{ false };


	/** Indicates if the character can crouch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Movement|Essential Variables")
	bool bCanCrouch = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	/** Animation instance of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBaseAnimInstance_ABP* AnimInstance;
};
