// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LegionCore : ModuleRules
{
	public LegionCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] {});
		PrivateIncludePaths.AddRange(new string[] {});
			
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
		});
		
		DynamicallyLoadedModuleNames.AddRange(new string[] {});
	}
}
