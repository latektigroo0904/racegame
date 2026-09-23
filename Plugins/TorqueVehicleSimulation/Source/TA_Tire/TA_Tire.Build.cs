using UnrealBuildTool;

public class TA_Tire : ModuleRules
{
    public TA_Tire(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "TA_Core",
                "TA_Surface"
            }
        );
    }
}
