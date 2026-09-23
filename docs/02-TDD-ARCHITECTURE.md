# Torque Atlas — Technical Architecture

## Engine strategy
Use Unreal Engine 5.x for:
- rendering;
- world streaming;
- asset pipeline;
- UI;
- input;
- audio;
- editor;
- high-level networking framework.

Build simulation-heavy systems as custom C++.

## Project tree
```
TorqueAtlas/
├── Config/
├── Content/
├── Plugins/
├── Source/
├── Tests/
├── Tools/
└── Documentation/
```

## C++ modules
```
TA_Core
TA_Vehicle
TA_Tire
TA_Powertrain
TA_Structure
TA_Damage
TA_Surface
TA_Telemetry
TA_World
TA_Traffic
TA_Network
TA_Editor
```

## Primary plugin
```
Plugins/TorqueVehicleSimulation/
├── Runtime/
├── Editor/
├── Tests/
└── Content/
```

## Vehicle composition
```
ATAVehiclePawn
├── UTAChassisComponent
├── UTAStructureComponent
├── UTASuspensionComponent
├── UTASteeringComponent
├── UTAPowertrainComponent
├── UTABrakeComponent
├── UTATireSystemComponent
├── UTAAeroComponent
├── UTADamageComponent
├── UTAThermalComponent
└── UTATelemetryComponent
```

Pawn orchestrates components; it should not contain most physics logic.

## Data-driven vehicle definition
No physics constants hard-coded per car.

```
UTAVehicleDefinition
├── Identity
├── Dimensions
├── Mass / CenterOfMass
├── Chassis
├── Suspension
├── Steering
├── Wheels / Tires
├── Brakes
├── Engine
├── Transmission
├── Differential
├── Aerodynamics
├── Cooling
├── Structure
└── Damage
```

## Instance vs definition
Definition = original vehicle specification.
Instance = owner, mileage, installed parts, damage, wear, fuel, tires, history.

## Units
Physics layer uses SI:
- metre;
- kilogram;
- second;
- Newton;
- Newton-metre;
- Pascal/kPa;
- Celsius/Kelvin where appropriate.

Explicit conversion helpers isolate Unreal centimetres from physics metres.

## Fixed-step strategy
Target ranges:
- render/game: 60–120 Hz;
- player vehicle: ~240 Hz;
- tire solver: ~240–480 Hz adaptive;
- structural solver normal: ~120 Hz;
- structural solver during severe impact: ~240–480 Hz adaptive;
- thermal: 20–60 Hz;
- wear/maintenance: 10–20 Hz.

All figures are provisional R&D targets.

## Physics substep order
1. Read processed controls.
2. Update powertrain.
3. Calculate wheel kinematics.
4. Query road surfaces.
5. Solve tire forces.
6. Solve suspension.
7. Solve braking.
8. Solve aero.
9. Integrate chassis dynamics.
10. Solve structural constraints.
11. Detect damage events.
12. Update mechanical damage.
13. Update thermal state.
14. Store telemetry.

## Input path
```
Device
→ Raw Input
→ Input Processor
→ Assist Layer
→ Driver Controls
→ Vehicle Solver
```

Assists modify driver input, not the underlying laws of physics.

## Debug/telemetry
Every solver exposes:
- timing;
- forces;
- state;
- warnings;
- deterministic test hooks.

Telemetry fields include speed, RPM, gear, control inputs, acceleration, yaw, wheel loads, slips, tire/brake temperatures, damage events and structural stress.

## Testing
Unit tests:
- gear ratios;
- torque interpolation;
- differential conservation;
- braking;
- slip;
- thermal energy;
- structural yield/fracture.

Automated driving tests:
- 0–100 km/h;
- 100–0 braking;
- constant-radius;
- slalom;
- crash regression.

## Multiplayer readiness
Do not replicate all structural nodes every frame.

Authoritative state focuses on:
- transform/velocities;
- powertrain;
- wheel state;
- critical mechanical damage;
- impact events;
- periodic structural corrections/checksums.

Visual deformation may be reconstructed locally while functional damage remains server authoritative.
