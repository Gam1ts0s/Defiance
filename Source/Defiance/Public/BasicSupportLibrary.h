// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BasicSupportLibrary.generated.h"

/**
 * 
 */
UCLASS()
class DEFIANCE_API UBasicSupportLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// Returns a vector pointing to the direction of the players movement input on the XY axis only (on World Transform)
	UFUNCTION(BlueprintCallable)
	static FVector GetPlayerInputMovementDirectionXY(FRotator CameraRotation, float ForwardMovementValue, float RightMovementValue);

	// Returns a vector pointing to the direction of the players movement input (on World Transform)
	UFUNCTION(BlueprintCallable)
	static FVector GetPlayerInputMovementDirection(FRotator CameraRotation, float ForwardMovementValue, float RightMovementValue);

	// Returns the length of a vector
	UFUNCTION(BlueprintCallable)
	static float GetVectorLength(FVector InputVector);

	// Returns the angle between the 2 vectors
	UFUNCTION(BlueprintCallable)
	static float GetAngleBetweenVectors(FVector VecA, FVector VecB);
};
