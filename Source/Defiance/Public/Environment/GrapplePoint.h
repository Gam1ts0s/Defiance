// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Grapple.h"
#include "GrapplePoint.generated.h"

UCLASS()
class DEFIANCE_API AGrapplePoint : public AActor, public IGrapple
{
	GENERATED_BODY()



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Sets default values for this actor's properties
	AGrapplePoint();

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	APawn* PlayerPawnRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PlayerInteractRange{ 0.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PlayerDetectionRange{ 0.0f };


	UPROPERTY(EditAnywhere)
	float MinDistanceToPlayer{ 200.0f };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector LandingLocation{ FVector::ZeroVector };



	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsActive{ false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DistanceToPlayer;

	
	UFUNCTION(BlueprintCallable)
	void UpdateDistanceToPlayer();

	UFUNCTION(BlueprintCallable)
	void ActivateGrapplePoint(APawn* PlayerPawn, float InteractRange, float DetectionRange);

	UFUNCTION(BlueprintCallable)
	void DeactivateGrapplePoint();

	UFUNCTION(BlueprintCallable)
	virtual FVector GetLandingLocation() override;

};
