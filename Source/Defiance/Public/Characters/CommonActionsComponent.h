// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types.h"
#include "CommonActionsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(
	FOnUpdatedMovementStanceSignature,
	UCommonActionsComponent, OnUpdatedMovementStanceDelegate,
	EMovementStance, NewMovementStance
);



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEFIANCE_API UCommonActionsComponent : public UActorComponent
{
	GENERATED_BODY()

	ACharacter* OwnerRef;

	APlayerController* Controller;

	class UCharacterMovementComponent* MovementComp;

	FVector CameraBoomLocation;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	// Sets default values for this component's properties
	UCommonActionsComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// Property replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable)
	FOnUpdatedMovementStanceSignature OnUpdatedMovementStanceDelegate;


	/*-------------------------------------------LOCOMOTION-------------------------------------------*/
	/** Indicates whether the character should use directional movement */
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Movement|Locomotion")
	bool bUseDirectionalMovement = false;

	UFUNCTION(BlueprintCallable)
	void HandleUpdatedUseDirectionalMovement(bool bNewUseDirectionalMovement);

	/** Indicates the current movement stance of the pawn */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_MovementStance,Category = "Movement|Locomotion")
	EMovementStance MovementStance = EMovementStance::Running;

	UFUNCTION()
	void OnRep_MovementStance();

	/*----------------------------------------------JUMP---------------------------------------------*/
	/** Indicates if the character can jump */
	UPROPERTY(EditAnywhere, Replicated, Category = "Movement|Jump")
	bool bCanJump = true;

	UFUNCTION(BlueprintCallable)
	void Jump();

	UFUNCTION(BlueprintCallable)
	void StopJumping();

	/*---------------------------------------------SPRINT--------------------------------------------*/
	/** Indicates if the character can sprint */
	UPROPERTY(EditAnywhere, Replicated, Category = "Movement|Sprint")
	bool bCanSprint = true;

	/** Indicates if the character is sprinting */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsSprinting, Category = "Movement|Sprint")
	bool bIsSprinting = false;

	/** The maximum speed the pawn can move when walking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	float MaxRunSpeed = 500.f;

	/** The maximum speed the pawn can move when walking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	float MaxSprintSpeed = 1000.f;

	UFUNCTION(BlueprintCallable)
	void ToggleSprint();

	/** Elevates maximum movement speed of the pawn to sprint speed */
	UFUNCTION(Server, Reliable, Blueprintcallable)
	void SR_StartSprint();

	/** Called when the Sprint/Dodge button is released */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SR_EndSprint();

	/** Called on both Server (Manually) and Clients (Automatically) when bIsSprinting is replicated */
	UFUNCTION()
	void OnRep_IsSprinting();

	/*---------------------------------------------CROUCH--------------------------------------------*/

	/** Indicates if the character can crouch */
	UPROPERTY(EditAnywhere, Replicated, Category = "Movement|Crouch")
	bool bCanCrouch = true;

	/** Indicates if the character is crouching */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsCrouching, Category = "Movement|Crouch")
	bool bIsCrouching = false;

	/** The maximum speed the pawn can move when crouched */
	UPROPERTY(EditAnywhere, Category = "Movement|Crouch")
	float MaxCrouchSpeed = 200.f;

	UFUNCTION(BlueprintCallable)
	void ToggleCrouch();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SR_StartCrouch();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SR_EndCrouch();

	UFUNCTION()
	void OnRep_IsCrouching();

	/*-------------------------------------------DODGE/ROLL------------------------------------------*/
	/** Indicates if the character can dodge */
	UPROPERTY(EditAnywhere, Replicated, Category = "Movement|Dodge & Roll")
	bool bCanDodge = true;

	/** Indicates if the character is dodging */
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Movement|Dodge & Roll")
	bool bIsDodging = false;

	/** Indicates if the character can roll */
	UPROPERTY(EditAnywhere, Replicated, Category = "Movement|Dodge & Roll")
	bool bCanRoll = true;

	/** Indicates if the character is rolling */
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Movement|Dodge & Roll")
	bool bIsRolling = false;

	/** A collection of dodge animation montages mapped according to direction */
	UPROPERTY(EditAnywhere, Category = "Movement|Dodge & Roll")
	TMap<EDetailedDirection, UAnimMontage*> DodgeAnimMontage;

	/** A collection of roll animation montages mapped according to direction */
	UPROPERTY(EditAnywhere, Category = "Movement|Dodge & Roll")
	TMap<EDetailedDirection, UAnimMontage*> RollAnimMontage;

	UFUNCTION(BlueprintCallable)
	void DodgeRoll();

	UFUNCTION(Server, Reliable, WithValidation)
	void SR_Dodge(EDetailedDirection DetailedDirection);

	UFUNCTION(NetMulticast, Reliable)
	void NM_PlayDodgeAnim(EDetailedDirection DetailedDirection);

	UFUNCTION()
	void FinishDodgeAnim();

	UFUNCTION(Server, Reliable, WithValidation)
	void SR_Roll(EDetailedDirection DetailedDirection);

	UFUNCTION(NetMulticast, Reliable)
	void NM_PlayRollAnim(EDetailedDirection DetailedDirection);

	UFUNCTION()
	void FinishRollAnim();
	
};
