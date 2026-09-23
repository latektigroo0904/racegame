using UnrealBuildTool;

public class TA_Telemetry : ModuleRules
{
    public TA_Telemetry(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "TA_Vehicle"
            }
        );
    }
}
