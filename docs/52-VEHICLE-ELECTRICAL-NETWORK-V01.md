# Vehicle Electrical Power Network v0.1

Updated: 2026-10-08
Status: implementation-ready architecture; canonical promotion gated by UE Proof-of-Physics

## 1. Objective

Replace selected normalized electrical efficiency states with a physically traceable low-voltage vehicle power network.

Prototype target:
- 12 V ICE architecture for TA-P01.

Later backends:
- 48 V mild hybrid;
- high-voltage EV/hybrid traction network.

## 2. Core nodes

Initial 12 V graph:

```
Battery
├── Starter
├── Main Fuse / Main Bus
│   ├── ECU / Ignition / Injectors
│   ├── Fuel Pump
│   ├── Cooling Fan
│   ├── ABS/ESC Controller
│   └── Lighting/Accessories
└── Alternator charge path
```

## 3. Battery state

```
FTABatteryState
{
    StateOfCharge01
    OpenCircuitVoltageV
    TerminalVoltageV
    TemperatureC
    Health01
}
```

Simple internal-resistance model:

```
V_terminal =
V_oc
- I_total × R_internal
```

SOC integrates net current:

```
dQ/dt = -I
```

## 4. Starter load

Starter electrical power approximately relates to mechanical output and efficiency:

```
P_electric =
P_mechanical / eta_starter
```

Current:

```
I_starter =
P_electric / max(V_terminal, epsilon)
```

Large current draw causes voltage sag.

If voltage is too low:
- starter torque falls;
- ECU may brown out depending on threshold.

## 5. Alternator

Alternator output depends on:
- engine RPM;
- max current;
- regulator voltage;
- health;
- belt/drive state.

Mechanical alternator load is fed back to engine accessory torque.

No free electrical power.

## 6. Bus and consumer model

Each consumer defines:
- requested power/current;
- minimum operating voltage;
- nominal voltage;
- health;
- priority/criticality.

Examples:
- ECU;
- fuel pump;
- ABS module;
- cooling fan;
- headlights.

Below minimum voltage, consumer effectiveness degrades or disconnects.

## 7. ECU / ignition authority

Current `EngineControlEfficiency01` can later derive from:
- bus voltage;
- ECU connection state;
- ignition/injector supply;
- ECU health.

This preserves compatibility while moving source-of-truth to network state.

## 8. Fuel pump coupling

Fuel pump electrical supply influences:
- pump speed/pressure;
- fuel rail pressure;
- fuel-delivery authority.

This couples electrical and fluid networks through explicit actuator power rather than arbitrary shared damage.

## 9. Cooling fan coupling

Fan airflow depends on:
- bus voltage;
- controller command;
- fan health.

At low road speed, electrical failure can therefore reduce radiator cooling.

At high road speed, ram airflow may dominate.

## 10. Fuses and relays

First model:
- connection state;
- current limit;
- trip/blown state;
- contact resistance.

A fuse may blow from overcurrent.

A damaged relay/contact may add resistance or disconnect.

## 11. Wiring

Do not simulate every wire.

Critical harness edges only:
- battery main;
- starter;
- ECU;
- fuel pump;
- ABS;
- cooling fan.

Each edge:
- resistance;
- max current;
- connection health.

## 12. Crash damage

Structural bindings may cause:
- cable disconnect;
- short-to-ground;
- increased resistance;
- battery isolation;
- alternator disconnect.

The existing `ElectricalDisconnection` signal remains a useful adapter into network edge state.

## 13. Short circuit

A short path should create current draw limited by:
- battery internal resistance;
- harness resistance;
- fuse behavior.

Do not model a short as instant generic electrical death unless the main fuse/battery path actually trips/fails.

## 14. Battery damage

Possible consequences:
- reduced capacity;
- increased internal resistance;
- open circuit;
- later thermal runaway/fire for appropriate chemistries.

TA-P01 12 V lead-acid-like prototype does not require EV-style runaway.

## 15. Authoring

Vehicle definition:
- battery nominal capacity;
- internal resistance;
- alternator curve;
- consumer power demands;
- harness resistance/limits;
- voltage thresholds;
- fuse ratings.

PhysicsConfigHash includes persistent calibration.

Vehicle instance stores:
- SOC;
- battery aging/health;
- current faults.

## 16. Telemetry

- battery SOC;
- open-circuit voltage;
- terminal voltage;
- total current;
- starter current;
- alternator current;
- bus voltage;
- ECU supply;
- fuel-pump supply;
- cooling-fan supply;
- fuse states.

## 17. Tests

ELEC-001 no-load battery holds OCV.
ELEC-002 starter load causes voltage sag.
ELEC-003 higher internal resistance causes larger sag.
ELEC-004 low SOC lowers available voltage/energy.
ELEC-005 alternator recharges battery above idle threshold.
ELEC-006 alternator load feeds mechanical accessory torque.
ELEC-007 blown ECU fuse removes engine-control authority.
ELEC-008 fuel-pump undervoltage lowers fuel pressure path.
ELEC-009 cooling-fan undervoltage lowers fan airflow.
ELEC-010 short circuit trips correctly rated fuse.
ELEC-011 disconnected noncritical accessory does not kill engine.
ELEC-012 deterministic fixed-step result.

## 18. EV/hybrid extension boundary

High-voltage traction battery must be a separate backend with:
- pack voltage;
- cell/segment abstraction;
- inverter;
- motor;
- contactors;
- thermal management;
- regenerative power flow.

Do not stretch the 12 V starter/bus model into traction EV physics.

## 19. Promotion rule

Keep current normalized ElectricalBus functional damage as compatibility layer until the physical network reproduces starter/engine-control consequences and UE regression evidence is available.
