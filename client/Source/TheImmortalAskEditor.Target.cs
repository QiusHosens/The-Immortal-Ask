using UnrealBuildTool;
using System.Collections.Generic;

public class TheImmortalAskEditorTarget : TargetRules
{
	public TheImmortalAskEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("TheImmortalAsk");
	}
}