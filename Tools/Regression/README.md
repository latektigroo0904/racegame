# Regression Scenario Tooling

Versioned proving-ground scenario contracts for Torque Atlas.

A scenario describes:
- simulation cadence;
- vehicle definition;
- environment;
- control program;
- metrics and acceptance envelopes.

The Unreal runner may later consume or translate these manifests. The format exists now so test intent is versioned before executable traces arrive.

## Scenario identity

Every scenario has a stable namespaced ID:

```text
ta.scenario:<name>
```

## Acceptance envelopes

Each envelope identifies:
- metric;
- statistic;
- optional wheel index;
- minimum allowed;
- maximum allowed.

Changing an acceptance envelope requires a scenario version increment and review.
