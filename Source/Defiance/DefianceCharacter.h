// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types.h"
#include "Curves/CurveFloat.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h"
#include "DefianceCharacter.generated.h"

UCLASS(config=Game)
class ADefianceCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Other ADefianceCharacter within this sphere can be locked on to  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = TargetLock, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* TargetLockSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = TargetLock, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* TargetLockWidget;

	/** A linear curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timelines, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* LinearCurve;


public:
	ADefianceCharacter();

	// Property replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Keep this as clean as possible */
	virtual void Tick(float DeltaTime);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called when the Dodge/Roll button is pressed */
	void DodgeRoll();

	/** Called on the Server when the Dodge/Roll button is pressed */
	UFUNCTION(Server, Reliable)
	void SR_DodgeRoll();
	void SR_DodgeRoll_Implementation();

	/** Implements Dodge logic */
	UFUNCTION(Server, Reliable)
	void SR_Dodge();
	void SR_Dodge_Implementation();

	/** Implements Roll logic */
	UFUNCTION(Server, Reliable)
	void SR_Roll();
	void SR_Roll_Implementation();

	/** Elevates maximum movement speed of the pawn to sprint speed */
	UFUNCTION(Server, Reliable)
	void SR_StartSprint();
	void SR_StartSprint_Implementation();

	/** Called when the Sprint/Dodge button is released */
	UFUNCTION(Server, Reliable)
	void SR_EndSprint();
	void SR_EndSprint_Implementation();

	/** Locks/Unlocks character orientation on a target */
	void ToggleTargetLock();

	UFUNCTION(Server, Reliable)
	void SR_ToggleTargetLock();
	void SR_ToggleTargetLock_Implementation();

	UFUNCTION(Client, Reliable)
	void CL_ToggleTargetLockWidget(ADefianceCharacter* Target);
	void CL_ToggleTargetLockWidget_Implementation(ADefianceCharacter* Target);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/** Used to turn Pawn based on mouse input */
	void Turn(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Used to make Pawn look up/down based on mouse input */
	void LookUp(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);


	/** Attempts to select a target to lock on */
	ADefianceCharacter* SelectTargetToLockOn();

	UFUNCTION(Client, Reliable)
	void CL_PointCameraToTarget(AActor* Target);
	void CL_PointCameraToTarget_Implementation(AActor* Target);

	/** Reset camera to look the same direction as the character mesh */
	UFUNCTION(Client, Reliable)
	void CL_ResetCamera();
	void CL_ResetCamera_Implementation();

	/** Rotates camera towards LockedOnTarget */
	void RotateCamera();

	/** Returns a vector pointing to the direction of the players movement input on the XY axis only(on World Transform) */
	UFUNCTION(BlueprintCallable)
	FVector GetPlayerInputMovementDirectionXY();

	/** Returns a vector pointing to the direction of the players movement input(on World Transform) */
	UFUNCTION(BlueprintCallable)
	FVector GetPlayerInputMovementDirection();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Indicates if the character is locked on target */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsLockedOnTarget, Category = TargetLock)
	bool bIsLockedOnTarget = false;

	UFUNCTION()
	void OnRep_IsLockedOnTarget();

	/** The maximum speed the pawn can move when crouched */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Parameters")
	float MaxCrouchSpeed = 200.f;

	/** The maximum speed the pawn can move when walking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Parameters")
	float MaxWalkSpeed = 500.f;

	/** The maximum speed the pawn can move when walking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Parameters")
	float MaxSprintSpeed = 1000.f;

	/** Indicates whether the character should use directional movement */
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Movement|Essential Variables")
	bool bUseDirectionalMovement = false;

	/** Indicates the current movement stance of the pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Movement|Essential Variables")
	EMovementStance MovementStance = EMovementStance::Running;
	
	/** Indicates if the character can sprint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Movement|Essential Variables")
	bool bCanSprint = true;

	/** Indicates if the character is sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsSprinting, Category = "Movement|Essential Variables")
	bool bIsSprinting = false;

	/** Called on both Server (Manually) and Clients (Automatically) when bIsSprinting is replicated */
	UFUNCTION()
	void OnRep_IsSprinting();

	/** Holds the angle at which the pawn should dodge/roll when using directional movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Movement|Dodge & Roll")
	float DodgeRollAngle;

	UFUNCTION(Server, Reliable)
	void SR_UpdateDodgeRollAngle(float Angle);
	void SR_UpdateDodgeRollAngle_Implementation(float Angle);

	/** Indicates if the character can dodge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Movement|Dodge & Roll")
	bool bCanDodge = true;

	/** Indicates if the character is dodging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsDodging, Category = "Movement|Dodge & Roll")
	bool bIsDodging = false;

	/** Called on both Server (Manually) and Clients (Automatically) when bIsDodging is replicated */
	UFUNCTION()
	void OnRep_IsDodging();

	/** Indicates if the character can roll */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Movement|Dodge & Roll")
	bool bCanRoll = true;

	/** Indicates if the character is rolling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsRolling, Category = "Movement|Dodge & Roll")
	bool bIsRolling = false;

	/** Called on both Server (Manually) and Clients (Automatically) when bIsRolling is replicated */
	UFUNCTION()
	void OnRep_IsRolling();

	/** Triggers when characters that can be locked onto enter the TargetLockSphere */
	UFUNCTION()
	void OnTargetLockSphereBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Adds a new target in the LockableTargets array */
	UFUNCTION(NetMulticast, Reliable)
	void NM_AddNewLockableTarget(class AActor* Actor);
	void NM_AddNewLockableTarget_Implementation(class AActor* Actor);

	/** Triggers when characters that can be locked onto exit the TargetLockSphere */
	UFUNCTION()
	void OnTargetLockSphereEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Removes a target from the LockableTargets array */
	UFUNCTION(NetMulticast, Reliable)
	void NM_RemoveLockableTarget(class AActor* Actor);
	void NM_RemoveLockableTarget_Implementation(class AActor* Actor);


private:
	/** Animation instance of the character */
	UPROPERTY(VisibleAnywhere, Category = Animation, meta = (AllowPrivateAccess = "true"))
	class UBaseAnimInstance_ABP* AnimInstance;

	/** Array with candidate targets the character can lock on */
	UPROPERTY(VisibleAnywhere, Replicated, Category = TargetLock)
	TArray<ADefianceCharacter*> LockableTargets;

	/** The target the character is currently locked on */
	UPROPERTY(VisibleAnywhere, Replicated, Category = TargetLock)
	ADefianceCharacter* LockedOnTarget;

	/** Offset pitch angle for locked target view so the pawn is not in between the camera and the locked target */
	UPROPERTY(EditAnywhere, Category = TargetLock)
	float CameraLockPitchAngleOffset = -30.f;








	//---------------------TIMELINES---------------------//
public:
	//CAMERA ROTATION TIMELINE
	/** Timeline used for smooth Camera rotation transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timelines, meta = (AllowPrivateAccess = "true"))
	UTimelineComponent* CameraRotationTimeline;
	
	/** Variables used for Camera Rotation Timeline */
	FRotator CameraStartRotation, CameraEndRotation;
	
	/** Delegate function to be binded with our timeline update function */
	FOnTimelineFloat CameraRotationTimelineUpdate{};

	/** Delegate function to be binded with our timeline finshed function */
	FOnTimelineEvent CameraRotationTimelineEnd{};

	/** Camera Rotation Timeline update function */
	UFUNCTION()
	void OnCameraRotationTimelineUpdate(float Value);

	/** Camera Rotation Timeline finish function */
	UFUNCTION()
	void OnCameraRotationTimelineEnd();

	/** Play Camera Rotation Timeline from start at a specific Rate */
	UFUNCTION()
	void PlayCameraRotationTimeline(float Rate);
};


