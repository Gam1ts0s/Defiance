#pragma once

#include "CoreMinimal.h"
#include "Types.generated.h"

//Enum to hold pawn movement stance
UENUM(BlueprintType)
enum class EMovementStance : uint8
{
	Crouched	UMETA(DisplayName = "Crouched"),
	Walking		UMETA(DisplayName = "Walking"),
	Running		UMETA(DisplayName = "Running"),
	Sprinting	UMETA(DisplayName = "Sprinting")
};


//Enum to describe movement direction
UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	MD_F		UMETA(DisplayName = "Forward"),
	MD_B		UMETA(DisplayName = "Backward"),
	MD_R		UMETA(DisplayName = "Right"),
	MD_L		UMETA(DisplayName = "Left")
};

//Enum with all directions around a character (used for dodging)
UENUM(BlueprintType)
enum class EDetailedDirection : uint8
{
	Forward			UMETA(DisplayName = "Forward"),
	ForwardLeft		UMETA(DisplayName = "Forward Left"),
	ForwardRight	UMETA(DisplayName = "Forward Right"),
	Backward		UMETA(DisplayName = "Backward"),
	BackwardLeft	UMETA(DisplayName = "BackwardLeft"),
	BackwardRight	UMETA(DisplayName = "BackwardRight"),
	Left			UMETA(DisplayName = "Left"),
	Right			UMETA(DisplayName = "Right")
};

//Enum with all climbing stances
UENUM(BlueprintType)
enum class EClimbStance : uint8
{
	Hanging		UMETA(DisplayName = "Hanging"),
	Swinging	UMETA(DisplayName = "Swinging"),
	Narrow		UMETA(DisplayName = "Narrow")
};

//Enum with all climbing transitions
UENUM(BlueprintType)
enum class EClimbTransition : uint8
{
	None		UMETA(DisplayName = "None"),
	Ascent		UMETA(DisplayName = "Ascent"),
	Descent		UMETA(DisplayName = "Descent"),
	Pivot		UMETA(DisplayName = "Pivot"),
	Corner		UMETA(DisplayName = "Corner"),
	Jump		UMETA(DisplayName = "Jump")
};

UENUM(BlueprintType)
enum class EClimbableType : uint8
{
	Grab		UMETA(DisplayName = "Grab"),
	Ledge		UMETA(DisplayName = "Ledge"),
	Surface		UMETA(DisplayName = "Surface"),
	HighBar		UMETA(DisplayName = "HighBar"),
	Rope		UMETA(DisplayName = "Rope")
};


//Struct with velocity values on each direction used to blend animations
USTRUCT(BlueprintType, Blueprintable)
struct FVelocityBlend
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
		float Forward;
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
		float Backward;
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
		float Left;
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
		float Right;
};
