using UnrealBuildTool;

public class TorqueAtlas : ModuleRules
{
    public TorqueAtlas(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "TA_Core",
                "TA_Vehicle"
            }
        );
    }
}
