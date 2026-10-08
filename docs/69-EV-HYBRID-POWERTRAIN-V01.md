# EV & Hybrid Powertrain Architecture v0.1

Updated: 2026-10-08
Status: implementation-ready design

## 1. Principle

High-voltage traction energy is a separate physical network from the 12 V accessory/starter bus.

Do not extend the 12 V model into EV traction.

## 2. HV battery

State:
- SOC;
- pack voltage;
- current;
- temperature;
- health;
- charge/discharge power limits.

Config:
- nominal energy kWh;
- open-circuit voltage vs SOC;
- internal resistance;
- max current/power;
- thermal mass/cooling;
- cell/segment abstraction later.

## 3. Battery electrical model

First-order equivalent circuit:

```
V_terminal = V_oc(SOC,T) - I * R_internal(SOC,T)
P = V_terminal * I
```

Solve current consistently for requested pack power.

Clamp to charge/discharge current and voltage bounds.

## 4. Motor/inverter

Motor map:
- torque limit vs speed;
- efficiency vs speed/torque;
- regen limit vs speed;
- thermal derate.

Electrical/mechanical relation:

motoring:
```
P_electric = P_mechanical / efficiency
```

regen:
```
P_electric_charge = P_mechanical_abs * efficiency_regen
```

No free energy.

## 5. Inverter

Owns:
- current limit;
- voltage limit;
- efficiency;
- temperature;
- health.

Controller requests motor torque; inverter/motor/battery limit actual torque.

## 6. Reduction/differential

Motor may drive:
- single reduction;
- multi-speed gearbox;
- front/rear e-axle;
- individual wheel motors later.

Use same differential interface where mechanically appropriate.

## 7. Regenerative braking

Driver brake intent is allocated between:
- regen torque;
- hydraulic friction brakes.

Constraints:
- battery charge acceptance;
- motor regen limit;
- low motor speed;
- ABS wheel-slip control;
- stability requests;
- SOC/temperature.

Hydraulic brakes fill remaining requested deceleration.

## 8. ABS + regen

Under lock tendency:
- regen at affected axle/wheel must reduce rapidly;
- hydraulic ABS still modulates pressure;
- controller coordinates to avoid fighting.

No direct wheel speed correction.

## 9. Brake blending

Target total wheel braking torque:

```
T_total_request
= T_regen_actual + T_friction_request
```

Friction request compensates only within physical hydraulic capacity.

Pedal feel abstraction remains independent from actual blend.

## 10. Thermal

HV battery:
- I²R heat;
- cooling circuit.

Motor/inverter:
- map-based loss;
- coolant loop.

Thermal derate affects torque/power limits.

## 11. 12 V DC-DC

EV/hybrid uses DC-DC converter:
HV pack → 12 V bus.

It replaces alternator as 12 V charging source.

12 V battery may still exist.

DC-DC has finite power/efficiency and health.

## 12. Hybrid architectures

Support later:
- P0/P1 mild hybrid;
- P2 motor between engine/transmission;
- P3 transmission output;
- P4 axle e-motor;
- power-split/series hybrid.

Each is a graph of existing engine/motor/clutch/gear/energy components.

## 13. Engine start in hybrid

P2/P3 systems may use traction motor as engine starter.

No need for conventional starter if architecture lacks one.

## 14. SOC management

Hybrid supervisory controller chooses:
- engine on/off;
- charge sustaining;
- motor assist;
- regen;
- battery reserve.

Controller cannot exceed component power/thermal constraints.

## 15. Damage

- cell/pack disconnect;
- inverter failure;
- motor derate;
- coolant failure;
- HV contactor open;
- isolation fault;
- DC-DC failure.

High-voltage safety presentation can be modeled without instructing real-world repair behavior.

## 16. Telemetry

- pack SOC/voltage/current/power/temp;
- requested/actual motor torque;
- motor speed;
- inverter power/loss;
- regen power;
- friction brake torque;
- blend ratio;
- DC-DC output.

## 17. Tests

EV-001 no-load pack voltage.
EV-002 discharge voltage sag.
EV-003 motor power energy sign.
EV-004 regen increases SOC.
EV-005 SOC cannot exceed bounds.
EV-006 high SOC limits regen.
EV-007 thermal derate limits torque.
EV-008 hydraulic fill when regen unavailable.
EV-009 ABS can reduce regen.
EV-010 DC-DC power conserves energy within efficiency.
HYBRID-001 engine-off EV drive.
HYBRID-002 motor-assist engine drive.
HYBRID-003 charge sustaining.
HYBRID-004 restart transition bounded.

## 18. MVP recommendation

Prototype EV after ICE baseline:
- one rear/front motor;
- single-speed reduction;
- one pack;
- hydraulic + regen blend;
- DC-DC to 12 V.

Hybrid follows later.
