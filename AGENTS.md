# StayInk Agent Guidance

## Product invariant

- StayInk must preserve the current E-Ink display while allowing genuine low-power suspend and normal wake behavior.
- Disabling or preventing genuine suspend is not a valid implementation.

## Safety

- Safety is more important than convenience.
- Never modify Kindle notebook data unless explicitly requested.
- Never modify the Kindle root filesystem unless explicitly requested and justified.
- Start with read-only observation. Prefer reversible runtime experiments before persistent changes.
- Every device-changing operation needs a documented rollback or recovery path.
- Never assume behavior from another Kindle model or firmware applies to Kindle Scribe.

## Evidence and licensing

- Label claims as **FACT**, **INFERENCE**, or **UNKNOWN**. Cite Kindle-internals sources where possible.
- Record the source repository, commit SHA, code location, device model, firmware, confidence, and StayInk relevance using `docs/research/`.
- Treat event names, log labels, and third-party terminology as observations, not proof of stock component ownership or low-power residency.
- Scope negative findings to what was reviewed; do not turn “not found” into “does not exist.”
- Do not copy GPL or AGPL source into StayInk.
- Use external implementations only to understand behavior and architecture unless license compatibility has been explicitly verified.

## Engineering discipline

- Prefer the smallest implementation or experiment that can prove or disprove a hypothesis.
- Define observable success criteria before experiments or implementation.
- Do not perform unrelated refactors.
- Do not declare behavior working until it has been verified.
