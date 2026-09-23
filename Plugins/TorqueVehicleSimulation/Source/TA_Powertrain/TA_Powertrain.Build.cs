using UnrealBuildTool;

public class TA_Powertrain : ModuleRules
{
    public TA_Powertrain(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "TA_Core"
            }
        );
    }
}
