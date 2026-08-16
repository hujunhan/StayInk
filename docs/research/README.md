# Research Records

This directory keeps evidence separate from interpretations and open questions.

- `SOURCES.md` pins source revisions and records source limitations.
- `FACTS.md` is the canonical ledger for directly supported observations.
- `POWER.md`, `DISPLAY.md`, `PACKAGING.md`, and `SCRIBE.md` synthesize facts by research question.
- `HYPOTHESES.md` ranks testable candidate mechanisms.
- `UNKNOWNS.md` records unresolved questions.
- `DEVICE_PROBE.md` records the completed Phase 2A read-only target reconnaissance procedure and closure boundary.
- `HIDE_METADATA_EXPERIMENT.md` records the unexecuted Phase 4A bounded `WM_NAME` experiment design and its authorization gate.
- `GUARD_WATCHDOG_VALIDATION.md` records the Phase 4B guard implementation, host audit, and unexecuted target no-write validation gates.

## Classification

- **FACT:** directly supported by a cited source or observation, only within its stated scope.
- **INFERENCE:** a reasoned conclusion that goes beyond direct evidence.
- **UNKNOWN:** missing, conflicting, or unverified knowledge.

Each important entry records a stable ID, source and revision, evidence location, model and firmware scope, confidence, and StayInk relevance. `UNKNOWN` is a valid value. A project source establishes what that project implements or assumes; it does not automatically establish stock Kindle or Scribe behavior.

Issue reports and web pages are mutable. Cite their URL and retrieval date instead of inventing a commit SHA. Keep third-party repositories unmodified under `.research/repos/`; never copy their source into StayInk.
