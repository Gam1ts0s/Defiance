// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimpleWidget.generated.h"

/**
 * 
 */
UCLASS()
class DEFIANCE_API USimpleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Color")
	FColor ActiveColor = FColor::White;
	
};
