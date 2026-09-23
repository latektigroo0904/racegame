# Vehicle Data & Simulation-State Schema v0.1

Status: implementation-oriented specification
Updated: 2026-09-23

## 1. Architectural rule
Vehicle configuration, persistent ownership state and live simulation state are three different data domains and must never be conflated.

```
Vehicle Definition (immutable authored specification)
        ↓
Vehicle Instance (persistent owner/configuration state)
        ↓
Simulation State (high-frequency runtime state)
```

## 2. Unreal asset strategy
Use a native `UTAVehicleDefinition : UPrimaryDataAsset` as the authored root asset.

Reason:
- editor-friendly;
- supports Asset Manager identity/loading;
- versionable;
- Blueprint-readable without putting physics logic in Blueprint;
- can reference subordinate data assets for tires, powertrains and materials.

Runtime simulation structs remain plain C++/POD-style data where possible. The physics thread must not depend on mutable UObject state.

## 3. Definition identity
Minimum fields:

```
VehicleDefinitionId
SchemaVersion
PhysicsVersion
DamageModelVersion
BrandId
ModelId
GenerationId
VariantId
ModelYearStart
ModelYearEnd
VehicleClass
```

A content change that alters handling must increment PhysicsVersion or the relevant component version.

## 4. Physical definition

### Dimensions
```
Length_m
Width_m
Height_m
Wheelbase_m
TrackFront_m
TrackRear_m
GroundClearance_m
```

### Mass
```
DryMass_kg
FluidsMass_kg
DriverReferenceMass_kg
CenterOfMassLocal_m
PrincipalInertia_kgm2
InertiaProducts_kgm2
```

Do not derive the final inertia tensor from a generic box in production vehicles. A box approximation is permitted only for the earliest placeholder prototype.

## 5. Axles and corners
A vehicle contains 2..N axles and explicit wheel corners.

```
FTAAxleDefinition
- AxleId
- LocalX_m
- Track_m
- IsSteered
- IsDriven
- BrakeBiasContribution

FTAWheelCornerDefinition
- CornerId
- AxleId
- Side
- TireDefinitionId
- WheelDefinitionId
- SuspensionDefinition
- BrakeDefinition
- HubMass_kg
```

The architecture must support future 6x6/8x8 vehicles even though TA-P01 uses four wheels.

## 6. Suspension authored data
Each corner stores chassis-local hardpoints, not arbitrary "handling values".

Examples:
```
UpperArmChassisA
UpperArmChassisB
UpperArmUpright
LowerArmChassisA
LowerArmChassisB
LowerArmUpright
TieRodChassis
TieRodUpright
DamperChassis
DamperUpright
SpringAxis
BumpStop
DroopStop
```

Hardpoints may later be displaced by structural damage.

## 7. Steering definition
```
SteeringWheelTurnsLockToLock
SteeringRatio
RackTravel_m
RackCenterLocal
AckermannTarget
PowerAssistCurve
Friction
Compliance
```

## 8. Powertrain definition

### Engine
```
EngineType
Displacement_m3
CylinderCount
IdleRPM
RedlineRPM
LimiterRPM
CrankInertia_kgm2
TorqueMap
ThrottleResponse
FrictionMap
ThermalDefinition
ForcedInductionDefinition
```

### Clutch
```
MaxClampForce
EffectiveRadius_m
FrictionCoefficientCold
FrictionCoefficientHot
PlateInertia
ThermalMass
FadeCurve
WearCurve
```

### Gearbox
```
GearRatios[]
ReverseRatio
FinalDriveRatio
InputInertia
OutputInertia
ShiftTime
GearEfficiency
```

### Differential
```
DifferentialType
Preload_Nm
PowerLockFactor
CoastLockFactor
ViscousCoefficient
```

Prototype TA-P01 uses Open differential; fields remain future-compatible.

## 9. Brakes
Per corner:
```
DiscRadius_m
DiscMass_kg
PadMuCurve
CaliperPistonArea_m2
HydraulicRatio
MaxPressure_Pa
ThermalMass
CoolingCoefficient
WearCurve
```

## 10. Aerodynamics
```
ReferenceArea_m2
Cd
ClFront
ClRear
CenterOfPressureLocal
CoolingDrag
AeroElements[]
```

Each aero element may later have health/damage and angle state.

## 11. Cooling and fluids
Authored capacities:
```
FuelCapacity
OilCapacity
CoolantCapacity
BrakeFluidCapacity
TransmissionFluidCapacity
```

Cooling graph:
```
EngineHeatSource
Thermostat
Pump
Radiator
Fan
AmbientAir
```

Connections carry thermal/fluid parameters rather than only "health".

## 12. Structural definition
The vehicle definition references:
```
StructuralNodeSet
StructuralConstraintSet
MaterialProfiles
MountDefinitions
DamageBindings
RenderDeformationBinding
```

A DamageBinding maps physical structure to functional systems, e.g. radiator support nodes → radiator damage input.

## 13. Persistent Vehicle Instance
Saved independently from definition:

```
VehicleInstanceId
DefinitionId
DefinitionVersion
OwnerId
Odometer_m
EngineHours_s
InstalledParts[]
SetupOverrides
FluidStates
TireInstances[]
WearStates
DamagePersistence
RepairHistory[]
PurchaseHistory
CosmeticState
```

The save must preserve the original definition/version used so migration is explicit after physics updates.

## 14. Runtime Simulation State
High-frequency data must be compact and allocation-free during a physics step.

```
FTAVehicleSimState
- Pose
- LinearVelocity
- AngularVelocity
- ChassisAcceleration
- WheelStates[]
- EngineState
- ClutchState
- GearboxState
- DifferentialStates[]
- BrakeStates[]
- ThermalState
- CriticalDamageState
- StructuralStateHandle
- SimulationTick
```

## 15. Runtime rule
Never dereference editable UObjects from the high-frequency solver.

At vehicle initialization:
1. validate asset;
2. copy/compile authored data into immutable runtime configuration;
3. allocate simulation state;
4. solver reads compiled configuration + state.

## 16. Compiled vehicle configuration
Introduce:
```
FTAVehicleCompiledConfig
```

Purpose:
- SI-normalized values;
- contiguous arrays;
- resolved IDs/indices;
- precomputed lookup tables;
- no editor metadata;
- stable hash.

This becomes the actual physics input.

## 17. Configuration hash
Generate a stable `PhysicsConfigHash` from all handling-critical compiled fields.

Use cases:
- replay validation;
- multiplayer setup verification;
- regression test identity;
- save migration diagnostics.

## 18. Validation
Asset compilation fails on errors such as:
- mass <= 0;
- duplicate IDs;
- invalid gear ratios;
- wheel radius <= 0;
- missing tire definition;
- suspension hardpoint degeneracy;
- no valid driven path from engine to driven wheels;
- structural damage binding references missing node/component.

Warnings:
- implausible center of mass;
- excessive static ride-height mismatch;
- negative aero reference area;
- temperature ranges out of expected engineering bounds.

## 19. TA-P01 first compiled definition
Initial target:
- 1420 kg reference running mass;
- RWD;
- 2.0 L turbo inline-four;
- 6MT;
- open rear differential;
- double-wishbone front;
- multi-link rear;
- four independently simulated tire states.

All calibration numbers remain provisional.

## 20. Acceptance
Schema v0.1 is accepted when:
- TA-P01 can be represented without hard-coded car-specific physics constants;
- compiled config can be created without UObject access during simulation;
- definition/instance/state are clearly separable;
- a stable configuration hash can be produced;
- damaged suspension pickup locations can override authored geometry.
