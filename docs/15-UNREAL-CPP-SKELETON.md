# Unreal/C++ Repository Skeleton v0.1

Status: implementation baseline
Updated: 2026-09-23

## 1. Target
Create the smallest real Unreal project foundation that preserves the agreed module boundaries and can later host the Proof-of-Physics prototype.

Target engine documentation baseline: Unreal Engine 5.8.
Compilation status remains **unverified** until built against an installed UE 5.8 toolchain.

## 2. Root
```
TorqueAtlas.uproject
Config/
Content/
Plugins/
Source/
Tests/
Tools/
docs/
```

Generated folders such as `Binaries`, `Intermediate`, `DerivedDataCache`, `Saved` and IDE artifacts are ignored.

## 3. Project module
```
Source/TorqueAtlas/
├── Public/
├── Private/
│   └── TorqueAtlas.cpp
└── TorqueAtlas.Build.cs
```

Role:
- game bootstrap;
- high-level gameplay composition;
- no tire/powertrain/structural equations.

## 4. Simulation plugin
```
Plugins/TorqueVehicleSimulation/
├── TorqueVehicleSimulation.uplugin
└── Source/
    ├── TA_Core/
    ├── TA_Vehicle/
    ├── TA_Tire/
    ├── TA_Powertrain/
    ├── TA_Surface/
    ├── TA_Structure/
    ├── TA_Damage/
    └── TA_Telemetry/
```

Each module follows:
```
ModuleName/
├── Public/
├── Private/
│   └── ModuleNameModule.cpp
└── ModuleName.Build.cs
```

Editor-specific tooling will be added later as `TA_Editor`.

## 5. Dependency direction
Keep dependencies acyclic.

Recommended:
```
TA_Core
↑
├── TA_Surface
├── TA_Tire
├── TA_Powertrain
├── TA_Structure
└── TA_Telemetry

TA_Vehicle depends on:
TA_Core + TA_Surface + TA_Tire + TA_Powertrain

TA_Damage depends on:
TA_Core + TA_Structure

TorqueAtlas game module depends on high-level plugin APIs, not solver internals.
```

Refine dependency boundaries before adding cross-module shortcuts.

## 6. Core public types
Initial code should establish:
- SI unit/conversion policy;
- version structs;
- stable IDs;
- compiled configuration interfaces;
- simulation tick index;
- validation result/error types.

## 7. Vehicle asset
First authored Unreal asset class:
```
UTAVehicleDefinition : UPrimaryDataAsset
```

It contains authored structs only. Before simulation it compiles into non-UObject runtime configuration.

## 8. Threading boundary
Rules:
- UObject/editor assets are read on initialization/compile path;
- inner physics loops operate on native runtime structs;
- no asset lookup, heap allocation or Blueprint call inside the high-frequency vehicle solver.

## 9. Tests
Use two layers:
1. pure/native low-level tests where practical;
2. Unreal Automation Framework for project/module/system tests.

Smoke tests must stay very fast; heavier proving-ground regression tests are separate.

## 10. Build verification checklist
When UE 5.8 is available:
1. generate project files;
2. build Development Editor;
3. launch editor;
4. verify all plugin modules load;
5. create a `UTAVehicleDefinition` asset;
6. run initial automation test;
7. run command-line test where possible;
8. record compiler/toolchain version and commit.

## 11. Do-not-do list
- no vehicle equations in Blueprint;
- no hard-coded TA-P01 constants in solver classes;
- no circular module dependencies;
- no runtime reliance on editor-only modules;
- no committed Unreal generated caches/binaries;
- no claim of compilation success before actual build.

## 12. Next code order
1. project/plugin descriptors;
2. TA_Core module;
3. vehicle definition schema headers;
4. compiled config/validation;
5. powertrain native structs;
6. tire native structs;
7. basic automation tests;
8. only then solver implementations.
