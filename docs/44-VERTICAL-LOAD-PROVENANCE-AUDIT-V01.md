# Vertical Load Provenance Audit V0.1

Updated: 2026-09-24
Status: source audit complete; executable verification pending.

## Purpose
Freeze the ownership and sign convention of `VerticalLoadN` used by P1.1 static-load evidence, and trace it from contact resolution through chassis force application. This audit is evidence-only and does not alter the physics model.

## Canonical convention
`VerticalLoadN` is a non-negative support-load magnitude in newtons. Direction is represented separately by the road/contact normal through `SuspensionForceWorldN = RoadNormalWorld * VerticalLoadN`. Consumers must not negate or absolute-value the scalar a second time.

## Provenance trace

### Geometric double-wishbone path
`ResolveDoubleWishboneRoadContact` obtains `SuspensionForce.TotalForceN`, clamps it once with `Max(0, TotalForceN)`, stores that magnitude in `VerticalLoadN`, then constructs the world support force as road normal times that magnitude.

### Compliant tire/suspension path
`EvaluateCompliantTravel` computes tire normal force and suspension reaction in the same positive support-load convention. The root residual is `TireVertical.NormalForceN - Max(0, SuspensionForce.TotalForceN)`. On the accepted contact, `VerticalLoadN` is assigned directly from `TireVertical.NormalForceN`, and `SuspensionForceWorldN` is constructed from road normal times that magnitude.

### Anti-roll coupling
The non-compliant pair path adds signed anti-roll load adjustments to each scalar and clamps the final corner magnitude at zero. The compliant axle path instead feeds the anti-roll reaction into the compliant root as `AdditionalSuspensionReactionN`; the final tire normal force remains the authoritative non-negative `VerticalLoadN`.

### Vehicle-contact bridge
`BuildVehicleWheelContactInput` copies `Contact.VerticalLoadN` and `Contact.SuspensionForceWorldN` without sign conversion.

### Tire and chassis consumers
`TAVehicleSimulation::BuildTireInput` copies `Contact.VerticalLoadN` directly into tire input. Chassis integration separately applies `Contact.SuspensionForceWorldN` at the suspension/contact application point. Therefore scalar tire capacity and vector chassis support force share one magnitude source without a second sign normalization.

### P1.1 evidence consumer
`TAStaticLoadEvidenceAdapter` must preserve the four contact magnitudes exactly. Negative or non-finite values are invalid evidence; they are not repaired with `Abs()`.

## Defect discovered during audit: compliant midpoint solve
The midpoint call to `EvaluateCompliantTravel` inside `ResolveDoubleWishboneCompliantRoadContact` omits the `AdditionalSuspensionReactionN` argument even though the helper signature requires it and the low/high endpoint calls pass it. This is both a compile/API defect and, if hidden by a future overload/default, would break anti-roll reaction consistency during bisection.

Prepared correction: `patches/44-compliant-contact-midpoint-reaction.patch` inserts `AdditionalSuspensionReactionN` between `RackDisplacementM` and `DamageOffsets` in the midpoint call. The patch is intentionally minimal and must be landed before claiming the compliant contact path buildable.

## Required regressions after patch 44 lands
1. Compile-time/API closure: every `EvaluateCompliantTravel` call supplies the anti-roll reaction argument.
2. Zero-reaction equivalence: zero additional reaction preserves the uncoupled compliant result.
3. Positive-reaction monotonicity: increasing additional support reaction changes the equilibrium in the physically expected direction without producing negative `VerticalLoadN`.
4. Left/right anti-roll conservation: paired anti-roll adjustments remain equal-and-opposite before zero-load clamping.
5. Provenance invariant: `BuildVehicleWheelContactInput` preserves `VerticalLoadN` exactly and `SuspensionForceWorldN` has non-negative projection onto the road normal.
6. P1.1 end-to-end: adapter receives the same FL/FR/RL/RR magnitudes produced by the final resolved contacts.

## Risks
- All conclusions are source-level until UE 5.8 UBT/Automation executes.
- Zero-load clamping can destroy strict equal-and-opposite anti-roll conservation when a wheel unloads; tests must distinguish pre-clamp anti-roll output from final contact loads.
- Tire normal force and suspension force can differ during transient/compliance solving; only the accepted final compliant contact is valid P1.1 evidence.
- A road normal with invalid orientation must be rejected upstream; a positive scalar alone does not prove a physically valid world force.

## Acceptance
This audit is closed only when patch 44 is landed, UBT compiles the compliant resolver, the listed regressions pass, and a stationary four-wheel capture confirms that evidence magnitudes match final resolved contact loads without sign conversion.