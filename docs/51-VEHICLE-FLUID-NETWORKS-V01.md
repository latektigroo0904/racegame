# Vehicle Fluid Networks v0.1

Updated: 2026-10-08
Status: implementation-ready architecture; physical-network promotion gated by executable Proof-of-Physics

## 1. Objective

Move from normalized functional damage efficiencies toward traceable fluid mass, pressure, temperature and leak behavior where those states materially affect vehicle physics.

First supported domains:
- coolant;
- fuel;
- engine oil;
- brake fluid.

The model is not CFD. It is a lumped network with explicit reservoirs, lines, consumers and leaks.

## 2. Common abstraction

A fluid network is a directed graph:

```
Reservoir
→ Pump/Pressure Source
→ Line/Junction
→ Consumer
→ Return/Reservoir
```

Each node/edge may expose:
- fluid mass;
- pressure;
- temperature;
- effective flow resistance;
- leak area;
- health/damage state.

## 3. Generic state contract

```
FTAFluidNodeState
{
    MassKg
    PressurePa
    TemperatureC
}

FTAFluidEdgeState
{
    FlowRateKgPerSec
    LeakAreaM2
    Health01
}
```

The first implementation may use specialized compact structs per domain while preserving this semantic contract.

## 4. Leak model

For a pressure-driven leak, use an orifice-style approximation:

```
mass_flow =
Cd
× Area
× sqrt(2 × rho × max(DeltaP, 0))
```

Clamp to available mass per timestep.

Do not allow negative reservoir mass.

## 5. Coolant network

Existing radiator/coolant state becomes the seed.

Target nodes:
- expansion reservoir;
- water pump;
- engine jacket;
- thermostat;
- radiator;
- hoses.

Key outputs:
- coolant mass;
- coolant temperature;
- circulation effectiveness;
- radiator heat rejection;
- leak flow.

Failure examples:
- crushed radiator lowers airflow/heat transfer;
- punctured radiator leaks;
- hose rupture dumps coolant rapidly;
- failed water pump reduces circulation;
- stuck thermostat causes overheating.

## 6. Engine oil network

State:
- sump oil mass;
- oil temperature;
- gallery pressure;
- pump efficiency;
- leak rate;
- viscosity proxy.

Pressure seed:

```
P_oil =
PumpPressureTarget(RPM, viscosity)
× pump health
× oil availability
- network losses
```

Consequences:
- insufficient pressure increases bearing/friction damage;
- low oil mass increases temperature;
- severe loss may seize engine.

No single oil-health scalar should replace these states.

## 7. Fuel network

Nodes:
- tank;
- pickup;
- low/high-pressure pump depending on engine;
- rail;
- injectors.

State:
- fuel mass;
- feed pressure;
- pump health;
- leak flow;
- delivery fraction derived from pressure/mass availability.

Combustion authority can then derive from rail pressure instead of the current normalized delivery efficiency.

## 8. Fuel slosh

Do not model full free-surface fluid initially.

Use a compact pickup-availability state driven by:
- longitudinal acceleration;
- lateral acceleration;
- fuel level;
- tank baffle calibration.

Low fuel + sustained high-g may reduce pickup availability.

## 9. Brake fluid network

Owned primarily by hydraulic brake subsystem.

State:
- circuit pressure;
- fluid temperature;
- vapor fraction;
- contamination;
- leak state.

Line rupture produces pressure decay through physical leak flow/compliance.

## 10. Thermal coupling

Each fluid domain may exchange heat with:
- engine block;
- brakes;
- ambient air;
- radiator/heat exchanger;
- fuel tank/body.

Use lumped thermal masses and explicit conductances.

## 11. Damage mapping

Structural damage should map to concrete components:

```
radiator node displacement/fracture
→ radiator leak/crush

fuel tank puncture
→ leak area

oil pan impact
→ oil leak area

brake line fracture
→ hydraulic edge rupture
```

Typed generic `FluidPressureLoss` remains useful as an adapter/event, not the long-term source of truth when a physical network exists.

## 12. Fire risk boundary

Fuel fire is deferred from v0.1 runtime.

Future fire model must require:
- fuel vapor/liquid leak;
- ignition source;
- temperature/energy condition.

Do not trigger fire from generic damage severity.

## 13. Authoring

Vehicle definition should author:
- capacities;
- pump parameters;
- line/resistance calibration;
- heat capacities/conductances;
- leak-sensitive component bindings;
- fluid material properties.

All persistent physics calibration enters PhysicsConfigHash.

Initial fluid quantity in a vehicle instance is persistent instance state, not definition identity.

## 14. Telemetry

Per domain:
- fluid mass;
- pressure;
- temperature;
- pump efficiency;
- leak rate;
- cumulative leaked mass;
- critical starvation flag.

## 15. Tests

FLUID-001 mass conservation without leaks.
FLUID-002 leak reduces stored mass monotonically.
FLUID-003 leak cannot remove more mass than available.
FLUID-004 zero pressure difference gives zero pressure-driven flow.
FLUID-005 larger leak area increases flow.
FLUID-006 pump damage lowers pressure.
FLUID-007 oil starvation increases engine damage path.
FLUID-008 fuel pressure loss lowers combustion authority.
FLUID-009 coolant loss lowers cooling performance.
FLUID-010 brake line rupture lowers circuit pressure.
FLUID-011 thermal energy exchange sign sanity.
FLUID-012 deterministic fixed-step result.

## 16. Promotion sequence

1. Preserve current coolant implementation as baseline.
2. Add isolated generic leak/orifice helper.
3. Add oil network.
4. Add fuel network.
5. Migrate coolant to common semantics where useful.
6. Connect brake fluid to hydraulic-brake subsystem.
7. Replace normalized functional efficiencies only after equivalent behavior is validated.

## 17. Non-goals

Deferred:
- CFD;
- detailed cavitation;
- fuel vapor chemistry;
- fire propagation;
- microscopic oil film;
- every individual hose/pipe;
- fluid sound/rendering.

## 18. Acceptance

A fluid network is accepted only when failure effects derive from mass/pressure/temperature/flow state and no duplicate scalar damage multiplier applies the same consequence again.
