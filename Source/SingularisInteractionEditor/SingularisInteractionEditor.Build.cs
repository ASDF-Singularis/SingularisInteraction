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

				"SingularisInteraction",

				"UMG",
				"UMGEditor",

				"UnrealEd",
				"AssetTools",
				"ContentBrowser"
			]
		);
	}
}