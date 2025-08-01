// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicSupportLibrary.h"


FVector UBasicSupportLibrary::GetPlayerInputMovementDirectionXY(FRotator CameraRotation, float ForwardMovementValue, float RightMovementValue)
{
	//Determine the direction the player is pointing based on the camera view and the movement axis
	FVector ForwardMovementVector;
	FVector RightMovementVector;
	FRotator MovementRotation;
	FVector MovementVector;

	MovementRotation = FRotator(0.f, CameraRotation.Yaw, 0);

	ForwardMovementVector = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X);
	RightMovementVector = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
	MovementVector = ForwardMovementVector * ForwardMovementValue + RightMovementVector * RightMovementValue;

	MovementVector.Normalize();

	return MovementVector;
}

FVector UBasicSupportLibrary::GetPlayerInputMovementDirection(FRotator CameraRotation, float ForwardMovementValue, float RightMovementValue)
{
	//Determine the direction the player is pointing based on the camera view and the movement axis
	FVector ForwardMovementVector;
	FVector RightMovementVector;
	FRotator MovementRotation;
	FVector MovementVector;

	MovementRotation = FRotator(CameraRotation.Pitch, CameraRotation.Yaw, 0);

	ForwardMovementVector = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X);
	RightMovementVector = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
	MovementVector = ForwardMovementVector * ForwardMovementValue + RightMovementVector * RightMovementValue;

	MovementVector.Normalize();

	return MovementVector;
}

float UBasicSupportLibrary::GetVectorLength(FVector InputVector)
{
	return sqrtf(powf(InputVector.X, 2) + powf(InputVector.Y, 2) + powf(InputVector.Z, 2));
}

float UBasicSupportLibrary::GetAngleBetweenVectors(FVector VecA, FVector VecB)
{
	FVector VectorA = VecA;
	FVector VectorB = VecB;
	VectorA.Normalize();
	VectorB.Normalize();
	return FMath::RadiansToDegrees(acosf(FVector::DotProduct(VectorA, VectorB)));
}

EDetailedDirection UBasicSupportLibrary::GetDetailedDirectionFromAngle(float Angle)
{
	//Forward
	if (Angle <= 22.5f && Angle > -22.5f) {
		return EDetailedDirection::Forward;
	}
	//Forward Right
	else if (Angle >= 22.5f && Angle < 67.5f) {
		return EDetailedDirection::ForwardRight;
	}
	//Right
	else if (Angle >= 67.5f && Angle < 112.5) {
		return EDetailedDirection::Right;
	}
	//Backward Right
	else if (Angle >= 112.5f && Angle < 157.5f) {
		return EDetailedDirection::BackwardRight;
	}
	//Backward
	else if (Angle <= -157.5f || Angle >= 157.5f) {
		return EDetailedDirection::Backward;
	}
	//Backward Left
	else if (Angle <= -112.5f && Angle > -157.5f) {
		return EDetailedDirection::BackwardLeft;
	}
	//Left
	else if (Angle <= -67.5f && Angle > -122.5f) {
		return EDetailedDirection::Left;
	}
	//Forward Left
	else if (Angle <= -22.5f && Angle > -67.5f) {
		return EDetailedDirection::ForwardLeft;
	}

	return EDetailedDirection::Forward;
}


