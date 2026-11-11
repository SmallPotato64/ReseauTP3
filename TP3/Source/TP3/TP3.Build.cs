// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TP3 : ModuleRules
{
	public TP3(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TP3",
			"TP3/Variant_Platforming",
			"TP3/Variant_Platforming/Animation",
			"TP3/Variant_Combat",
			"TP3/Variant_Combat/AI",
			"TP3/Variant_Combat/Animation",
			"TP3/Variant_Combat/Gameplay",
			"TP3/Variant_Combat/Interfaces",
			"TP3/Variant_Combat/UI",
			"TP3/Variant_SideScrolling",
			"TP3/Variant_SideScrolling/AI",
			"TP3/Variant_SideScrolling/Gameplay",
			"TP3/Variant_SideScrolling/Interfaces",
			"TP3/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
