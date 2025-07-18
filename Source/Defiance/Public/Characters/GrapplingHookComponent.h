// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapplingHookComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEFIANCE_API UGrapplingHookComponent : public UActorComponent
{
	GENERATED_BODY()

	ACharacter* OwnerRef;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	// Sets default values for this component's properties
	UGrapplingHookComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AActor* ActiveGrapple;

	UPROPERTY(EditAnywhere)
	float DetectionRadius{ 1000.0f };

	UPROPERTY(EditAnywhere)
	float InteractRange{ 600.0f };

	UPROPERTY(VisibleAnywhere)
	bool bCanLaunch{ true };

	UPROPERTY(VisibleAnywhere)
	bool bIsLaunching{ false };

	UPROPERTY(EditAnywhere)
	float ArcParam{ 0.3f };

	UPROPERTY(EditAnywhere)
	FVector LaunchModifier{ FVector(1.6, 1.6, 1.2) };


	UFUNCTION(BlueprintCallable)
	void DetectGrapple(float Range);

	void UpdateActiveGrapple(AActor* NewGrapple);

	UFUNCTION(BlueprintCallable)
	void LaunchOnGrapple();


	
};
