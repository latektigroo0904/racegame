using UnrealBuildTool;

public class TA_Core : ModuleRules
{
    public TA_Core(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject"
            }
        );
    }
}
