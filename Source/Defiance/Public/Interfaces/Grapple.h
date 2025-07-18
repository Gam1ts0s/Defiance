// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Grapple.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGrapple : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DEFIANCE_API IGrapple
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintImplementableEvent)
	void OnActivate(const APawn* PlayerPawnRef, float InteractRange, float DetectionRange);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeactivate();

	UFUNCTION(BlueprintImplementableEvent)
	void OnInteract();


	virtual FVector GetLandingLocation() { return FVector::ZeroVector; }

};
