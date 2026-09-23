using UnrealBuildTool;

public class TA_Vehicle : ModuleRules
{
    public TA_Vehicle(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "TA_Core",
                "TA_Surface",
                "TA_Tire",
                "TA_Powertrain"
            }
        );
    }
}
