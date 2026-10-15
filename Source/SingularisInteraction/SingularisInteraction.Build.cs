using UnrealBuildTool;

public class SingularisInteraction : ModuleRules
{
	public SingularisInteraction(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",
				"NetCore",
				"Projects",

				"UMG",
				"Slate",
				"SlateCore",

				"InputCore",
				"EnhancedInput",

				"GameplayTags",

				"AIModule",

				"EngineSettings",
				"DeveloperSettings"
			]
		);
	}
}