using UnrealBuildTool;

public class TheImmortalAsk : ModuleRules
{
	public TheImmortalAsk(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"HTTP",
			"Json",
			"JsonUtilities",
			"Slate",
			"SlateCore",
		});
	}
}
