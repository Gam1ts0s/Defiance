// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/GrapplePoint.h"
#include "GameFramework/Character.h"
#include "Characters/GrapplingHookComponent.h"


// Sets default values
AGrapplePoint::AGrapplePoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGrapplePoint::BeginPlay()
{
	Super::BeginPlay();
	
	//PlayerPawnRef = GetWorld()->GetFirstPlayerController()->GetPawn();
	
	
}

// Called every frame
void AGrapplePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive) { UpdateDistanceToPlayer(); }
}

void AGrapplePoint::UpdateDistanceToPlayer()
{
	FVector PlayerLocation{ PlayerPawnRef->GetActorLocation() };
	FVector CurrentLocation{ GetActorLocation() };
	DistanceToPlayer = static_cast<float>(FVector::Distance(CurrentLocation, PlayerLocation));
}

void AGrapplePoint::ActivateGrapplePoint(APawn* PlayerPawn, float InteractRange, float DetectionRange)
{
	if (!IsValid(PlayerPawn)) { return; }

	PlayerPawnRef = PlayerPawn;
	PlayerInteractRange = InteractRange;
	PlayerDetectionRange = DetectionRange;
	bIsActive = true;
}

void AGrapplePoint::DeactivateGrapplePoint()
{
	PlayerPawnRef = nullptr;
	PlayerInteractRange = 0.0f;
	PlayerDetectionRange = 0.0f;
	bIsActive = false;
}

FVector AGrapplePoint::GetLandingLocation()
{
	return  GetActorLocation() + LandingLocation;
}

