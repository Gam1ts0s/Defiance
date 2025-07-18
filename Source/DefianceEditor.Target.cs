// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DefianceEditorTarget : TargetRules
{
	public DefianceEditorTarget(TargetInfo Target) : base(Target)
	{
        bOverrideBuildEnvironment = true;
        CppStandard = CppStandardVersion.Cpp20;

        Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("Defiance");
	}
}
