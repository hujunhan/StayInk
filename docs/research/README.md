# Research Records

This directory keeps StayInk research evidence separate from conclusions and open questions.

- `SOURCES.md` registers exact source revisions.
- `FACTS.md` records directly supported observations.
- `HYPOTHESES.md` records testable explanations or proposed behavior.
- `UNKNOWNS.md` records unresolved questions that affect the project.

## Classification

- **FACT:** directly supported by a cited source or repeatable observation, within the recorded device and firmware scope.
- **INFERENCE:** a reasoned interpretation that goes beyond direct evidence. Record it as a hypothesis.
- **UNKNOWN:** missing or conflicting knowledge. Do not silently fill it with an assumption.

## Required metadata

Every research entry must include:

- a stable record ID;
- source ID and repository;
- exact commit SHA;
- file, function, symbol, path, or document section;
- Kindle model and firmware when known;
- confidence (`high`, `medium`, or `low`);
- relevance to StayInk;
- enough evidence or reasoning to audit the claim.

Write `UNKNOWN` when metadata cannot be established. Keep claims narrow enough that their cited evidence supports them. Use `.research/repos/` only for local, unmodified reference checkouts; do not copy third-party code into StayInk.
