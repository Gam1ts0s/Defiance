// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Defiance : ModuleRules
{
	public Defiance(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
