using UnrealBuildTool;
using System.Collections.Generic;

public class TorqueAtlasEditorTarget : TargetRules
{
    public TorqueAtlasEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("TorqueAtlas");
    }
}
