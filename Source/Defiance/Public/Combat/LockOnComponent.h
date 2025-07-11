// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(
	FOnUpdatedTargetSignature,
	ULockOnComponent, OnUpdatedTargetDelegate,
	AActor*, NewTargetActorRef
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEFIANCE_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

	ACharacter* OwnerRef;

	APlayerController* Controller;

	class UCharacterMovementComponent* MovementComp;

	class USpringArmComponent* CameraBoom;


public:	
	// Sets default values for this component's properties
	ULockOnComponent();

	// Property replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentTargetActor)
	AActor* CurrentTargetActor;

	UFUNCTION()
	void OnRep_CurrentTargetActor();

	UPROPERTY(BlueprintAssignable)
	FOnUpdatedTargetSignature OnUpdatedTargetDelegate;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	bool StartLockOn(float SphereRadious = 750.0f);

	UFUNCTION(BlueprintCallable)
	void EndLockOn();

	UFUNCTION(BlueprintCallable)
	void ToggleLockOn(float SphereRadious = 2000.0f);

	UFUNCTION(BlueprintCallable)
	void ResetCamera();

	UFUNCTION(Server, Reliable, WithValidation)
	void SR_UpdateLockOn(AActor* NewTarget);

	
	UPROPERTY(EditAnywhere)
	double BreakDistance{ 2000.0 };

	UPROPERTY(EditAnywhere)
	float CameraPitchCorrection{ -30.0f };


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
