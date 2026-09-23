using UnrealBuildTool;
using System.Collections.Generic;

public class TorqueAtlasTarget : TargetRules
{
    public TorqueAtlasTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("TorqueAtlas");
    }
}
