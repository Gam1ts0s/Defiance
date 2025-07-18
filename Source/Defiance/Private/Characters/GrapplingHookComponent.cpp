// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GrapplingHookComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interfaces/Grapple.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UGrapplingHookComponent::UGrapplingHookComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGrapplingHookComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerRef = GetOwner<ACharacter>();
	
}


// Called every frame
void UGrapplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (OwnerRef->IsLocallyControlled()) { DetectGrapple(DetectionRadius); }

}



void UGrapplingHookComponent::DetectGrapple(float Range)
{
	FVector CurrentLocation{ OwnerRef->GetActorLocation() };
	FCollisionShape Sphere{ FCollisionShape::MakeSphere(Range) };
	FCollisionQueryParams IgnoreParams{
		FName{TEXT("Ignore Collision Params")},
		false,
		OwnerRef
	};

	TArray<FHitResult> OutHit;

	bool bHasFoundTarget{ GetWorld()->SweepMultiByChannel(
		OutHit,
		CurrentLocation,
		CurrentLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		Sphere,
		IgnoreParams
	) };


	// Nothing detected. Deactivate previous detected target if there is one
	if (OutHit.Num() == 0) 
	{ 
		UpdateActiveGrapple(nullptr);
		return; 
	}

	UCameraComponent* CameraRef{ OwnerRef->GetComponentByClass<UCameraComponent>() };
	FVector CameraFwdVector{ CameraRef->GetForwardVector() };
	float FOV{ CameraRef->FieldOfView };

	AActor* SelectedTarget{ nullptr };
	float SelectedTargetDotProd{ -1.0f };

	for (const FHitResult &Hit : OutHit)
	{
		FHitResult VisibilityHit;

		bool bIsTargetVisible{ GetWorld()->LineTraceSingleByChannel(
			VisibilityHit,
			CameraRef->GetComponentLocation(),
			Hit.GetActor()->GetActorLocation(),
			ECollisionChannel::ECC_Visibility,
			IgnoreParams
		) };
			
		if (VisibilityHit.GetActor() == Hit.GetActor())
		{	
			FVector CameraToTargetDirection{ UKismetMathLibrary::GetDirectionUnitVector(
					CameraRef->GetComponentLocation(),
					Hit.GetActor()->GetActorLocation()
			) };

			float DotProd{ static_cast<float>(FVector::DotProduct(CameraFwdVector, CameraToTargetDirection)) };
			if (FMath::RadiansToDegrees(acosf(DotProd)) < FOV / 2)
			{
				
				if (DotProd > SelectedTargetDotProd)
				{
					SelectedTargetDotProd = DotProd;
					SelectedTarget = Hit.GetActor();
				}
			}
		}
	}

	// Nothing detected. Deactivate previous detected target if there is one
	if (!IsValid(SelectedTarget))
	{
		UpdateActiveGrapple(nullptr);
		return;
	}
	//Found new target
	else
	{
		// If the target is the same as the already activated target do nothing
		if (ActiveGrapple == SelectedTarget) { return; }

		// If the target is a new target deactivate previous target and activate the new one 
		UpdateActiveGrapple(SelectedTarget);
	}

}

void UGrapplingHookComponent::UpdateActiveGrapple(AActor* NewGrapple)
{
	// Deactivate previous active grapple point 
	if (IsValid(ActiveGrapple))
	{
		if (ActiveGrapple->Implements<UGrapple>())
		{
			IGrapple::Execute_OnDeactivate(ActiveGrapple);
		}
	}

	// Set new value and if it is valid activate the new active grapple point
	if (!IsValid(NewGrapple))
	{
		ActiveGrapple = nullptr;
	}
	else
	{
		ActiveGrapple = NewGrapple;
		if (!ActiveGrapple->Implements<UGrapple>()) { return; }
		IGrapple::Execute_OnActivate(ActiveGrapple, Cast<APawn>(OwnerRef), InteractRange, DetectionRadius);
	}
}

void UGrapplingHookComponent::LaunchOnGrapple()
{
	if (!IsValid(ActiveGrapple)) { return; }
	
	FVector CurrentLocation{ OwnerRef->GetActorLocation() };
	FVector GrappleLocation{ ActiveGrapple->GetActorLocation() };
	float DistanceToGrapple{ static_cast<float>(FVector::Distance(CurrentLocation, GrappleLocation)) };

	if (DistanceToGrapple > InteractRange) { return; }

	FVector LandingLocation;
	IGrapple* GrapplePoint{ Cast<IGrapple>(ActiveGrapple) };
	
	if (!GrapplePoint) { return; }
	
	LandingLocation = GrapplePoint->GetLandingLocation();

	FVector LaunchVelocity;
	bool bFoundVelocity = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		GetWorld(),
		LaunchVelocity,
		CurrentLocation,
		LandingLocation,
		0.0f,
		ArcParam
	);


	if (bFoundVelocity)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LaunchVelocity.ToString())
	}

}

