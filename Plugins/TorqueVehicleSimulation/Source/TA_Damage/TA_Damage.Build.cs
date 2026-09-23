using UnrealBuildTool;

public class TA_Damage : ModuleRules
{
    public TA_Damage(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "TA_Core",
                "TA_Structure"
            }
        );
    }
}
