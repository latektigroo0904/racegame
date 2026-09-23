using UnrealBuildTool;

public class TA_Structure : ModuleRules
{
    public TA_Structure(ReadOnlyTargetRules Target) : base(Target)
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
