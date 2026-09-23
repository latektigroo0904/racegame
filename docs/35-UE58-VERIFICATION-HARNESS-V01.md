# Unreal Engine 5.8 Verification Harness v0.1

Status: harness implemented; no UE build has been executed yet  
Updated: 2026-09-23

## 1. Purpose

Torque Atlas now has a reproducible verification entrypoint for the first real Unreal Engine build.

A source review, source-level Automation test, or GitHub source-sanity pass is **not** equivalent to a successful Unreal build.

The first trusted physics baseline requires all of:

1. Unreal Engine 5.8;
2. Development Editor compilation through UHT/UBT;
3. successful Editor module loading;
4. execution of the complete `TorqueAtlas.*` Automation namespace;
5. retained build/test logs and Automation report;
6. retained Git commit identity.

## 2. Source-only CI

Workflow:

`.github/workflows/source-sanity.yml`

Entry point:

`python Scripts/source_sanity.py`

It does not require Unreal Engine.

Checks include:

- merge-conflict marker lines;
- balanced C++ delimiters after stripping comments/literals;
- Unreal `.generated.h` include ordering;
- duplicate Automation test symbols;
- duplicate Automation test names;
- required project/plugin descriptors;
- exact expected plugin module set;
- module rules file presence;
- project EngineAssociation = 5.8.

The GitHub workflow also runs:

`bash -n Scripts/verify-unreal.sh`

to validate the Unix verification-runner syntax.

Passing this workflow means only that the repository passed these static checks.

## 3. Windows verification

Script:

`Scripts/Verify-Unreal.ps1`

Default engine discovery locations:

- `C:\Program Files\Epic Games\UE_5.8`
- `D:\Program Files\Epic Games\UE_5.8`
- `D:\Epic Games\UE_5.8`

Recommended invocation:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Verify-Unreal.ps1
```

Explicit engine path:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Verify-Unreal.ps1 -UERoot "D:\Epic Games\UE_5.8"
```

Optional development switches:

- `-SkipGenerateProjectFiles`
- `-SkipBuild`

These switches are for controlled reruns only; the first verification should run the full path.

## 4. Linux/macOS verification

Script:

`Scripts/verify-unreal.sh`

Recommended invocation:

```bash
bash Scripts/verify-unreal.sh --ue-root /path/to/UE_5.8
```

Or:

```bash
export UE_ROOT=/path/to/UE_5.8
bash Scripts/verify-unreal.sh
```

Supported hosts:

- Linux;
- macOS.

The script intentionally may be invoked through `bash` even if executable file mode has not been preserved by a content-API checkout.

## 5. Engine version gate

Both runners read:

`Engine/Build/Build.version`

and require:

```
MajorVersion = 5
MinorVersion = 8
```

A different engine version is rejected before compilation.

This prevents accidental creation of a “trusted” baseline against a different Unreal toolchain.

## 6. Build path

The full verification sequence is:

```
TorqueAtlas.uproject
→ optional GenerateProjectFiles
→ TorqueAtlasEditor Development build
→ UHT
→ UBT/C++ compiler
→ UnrealEditor-Cmd
→ Automation RunTests TorqueAtlas.
→ report + logs + metadata
```

Windows target:

- target: `TorqueAtlasEditor`
- platform: `Win64`
- configuration: `Development`

Unix target:

- target: `TorqueAtlasEditor`
- platform: host `Linux` or `Mac`
- configuration: `Development`

## 7. Automation command

The runners use the command namespace:

`Automation RunTests TorqueAtlas.`

and wait for:

`Automation Test Queue Empty`

The Editor is launched unattended with NullRHI so physics/unit regressions do not require normal rendering.

## 8. Verification artifacts

Each run writes under:

`Saved/Verification/<timestamp>/`

Expected contents include:

- `metadata.json`
- `generate-project-files.log` when generation runs
- `build.log`
- `automation.log`
- `automation-stdout.log`
- `AutomationReport/`

`Saved/` is ignored by Git.

The metadata record contains, where available:

- Git SHA;
- Unreal root;
- Unreal version;
- project;
- target;
- platform;
- configuration;
- Automation filter;
- build exit code;
- Automation exit code;
- final status.

## 9. Pass criteria

A run is considered a valid build-verification candidate only when:

- the exact engine gate passes;
- UHT/UBT build returns zero;
- UnrealEditor-Cmd returns zero after the Automation queue finishes;
- the Automation report contains the expected `TorqueAtlas.*` suite;
- no fatal/editor-load/module-load failures occurred;
- metadata identifies the tested Git SHA.

For the first trusted Proof-of-Physics baseline, additionally review:

- static-settle traces;
- acceleration/braking traces;
- constant-steer traces;
- asymmetric-road traces;
- synthetic front-corner crash traces;
- dynamic-unsprung experiment traces separately from canonical results.

## 10. Failure handling

A compiler or test failure is useful evidence and must not be hidden.

On failure:

1. preserve the run directory;
2. repair the smallest root cause;
3. commit the repair;
4. rerun against the new Git SHA;
5. never relabel a previous failed run as passed.

## 11. Current verification state

As of 2026-09-23:

- source-sanity workflow: configured;
- local UE runners: configured;
- Unreal Engine 5.8 build: **not yet executed in this environment**;
- Unreal Automation suite: **not yet executed**;
- trusted regression baseline: **not yet established**.

No document or release note should state that Torque Atlas currently compiles or passes UE Automation until those operations have actually completed.
