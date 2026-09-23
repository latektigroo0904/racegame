using UnrealBuildTool;

public class TA_Surface : ModuleRules
{
    public TA_Surface(ReadOnlyTargetRules Target) : base(Target)
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
