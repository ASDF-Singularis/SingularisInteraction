// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SingularisInteractionEditor : ModuleRules
{
	public SingularisInteractionEditor(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",
				"Projects",

				"UMG",
				"UMGEditor",

				"SingularisInteraction",

				"UnrealEd",
				"AssetTools",
				"ContentBrowser"
			]
		);
	}
}