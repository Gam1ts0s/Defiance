// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "MyInputConfigData.generated.h"

/**
 * Data asset containing all input actions a character can have
 */
UCLASS()
class DEFIANCE_API UMyInputConfigData : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputMove;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputLook;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputLookStick;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputJump;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputSprintDodge;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputSprint;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputDodgeRoll;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputCrouch;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputTargetLock;

};
