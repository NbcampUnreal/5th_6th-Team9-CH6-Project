// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StillBound : ModuleRules
{
	public StillBound(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
            "NavigationSystem",
            "AIModule",

			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "SlateCore",
            "MoviePlayer",

            "GameplayAbilities",
            "GameplayTasks",
            "GameplayTags"

        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"StillBound",
			"StillBound/Variant_Platforming",
			"StillBound/Variant_Platforming/Animation",
			"StillBound/Variant_Combat",
			"StillBound/Variant_Combat/AI",
			"StillBound/Variant_Combat/Animation",
			"StillBound/Variant_Combat/Gameplay",
			"StillBound/Variant_Combat/Interfaces",
			"StillBound/Variant_Combat/UI",
			"StillBound/Variant_SideScrolling",
			"StillBound/Variant_SideScrolling/AI",
			"StillBound/Variant_SideScrolling/Gameplay",
			"StillBound/Variant_SideScrolling/Interfaces",
			"StillBound/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
