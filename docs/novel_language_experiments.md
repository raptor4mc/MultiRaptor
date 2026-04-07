# Novel Language Experiments (MagPhos)

This page captures deliberately unusual language ideas requested by the community.
They are **experimental concepts** and are not part of the stable language contract.

## 1) Time-Travel Variables (`timeline`)
A variable that keeps historical snapshots and can be read from a specific logical tick.

```mp
timeline hp = 100
hp = hp - 10
hp = hp - 25

print hp@0   # 100
print hp@1   # 90
print hp@now # 65
```

Why it is novel:
- Most languages model history externally (event sourcing, logs, databases), not as first-class variable syntax.

## 2) Causality Guards (`because`)
Execution depends on explicit causal provenance of a value.

```mp
var score = read_sensor()

because score from "trusted.sensor.v2" {
  grant_reward(score)
} else {
  audit("untrusted score source")
}
```

Why it is novel:
- Typical languages track types, not source provenance as a first-class control primitive.

## 3) Counterfactual Blocks (`whatif`)
Run an alternate branch against a copied world-state and diff outcomes before commit.

```mp
world base = snapshot()

whatif base {
  spawn_boss("Hydra")
  give_player("plasma_spear")
} compare {
  print diff(world)
} commit_if (diff(world).fun_score > 0.8)
```

Why it is novel:
- Existing simulation tooling does this in frameworks, but not as a direct language-level statement with commit semantics.

## 4) Emotion-Tuned Diagnostics (`mood`)
Errors can be rendered with a user-selected communication style while preserving machine-readable codes.

```mp
mood diagnostics = "mentor"

fn divide(a, b) {
  if b == 0 {
    fail E_DIV_ZERO "Cannot divide by zero"
  }
  return a / b
}
```

Why it is novel:
- Languages typically separate compiler diagnostics from user-chosen tone/style contracts.

## 5) Multi-Reality Pattern Matching (`match all`)
Pattern match returns all valid interpretations when ambiguity is intentional.

```mp
match all parse("1/2/03") {
  case date_us(d)   => print("US", d)
  case date_eu(d)   => print("EU", d)
  case fraction(f)  => print("fraction", f)
}
```

Why it is novel:
- Mainstream pattern matching resolves to one branch; this model preserves ambiguity as data.

## 6) Compile-Time Negotiation (`negotiate`)
Compile steps can negotiate capabilities with runtime/platform targets.

```mp
negotiate runtime {
  require "deterministic_rng"
  prefer "simd128"
  fallback "portable_scalar"
}
```

Why it is novel:
- Capability detection is usually build-system specific; this treats it as language-level declarative intent.

---

## Suggested rollout strategy
1. Keep all features behind an `experimental` flag.
2. Ship parser-only prototypes with clear diagnostics before runtime semantics.
3. Add contract tests for deterministic behavior and downgrade/fallback paths.
4. Promote only features that survive at least one full minor cycle with telemetry and user feedback.
