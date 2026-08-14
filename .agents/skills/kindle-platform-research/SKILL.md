---
name: kindle-platform-research
description: Research and validate Kindle platform internals from external Kindle source repositories using pinned, license-aware evidence. Use when a task requires tracing Kindle power, display, input, packaging, lifecycle, or other system behavior; checking whether community-project behavior applies to a Kindle model or firmware; or turning source archaeology into documented facts, unknowns, hypotheses, and experiments.
---

# Kindle Platform Research

Investigate one concrete behavior without turning the source repositories into general summaries. Treat external projects as evidence of what those projects implement or observe, not as stock Kindle documentation.

## Workflow

1. Read the repository's `AGENTS.md`, `docs/SAFETY.md`, and existing `docs/research/` records before researching. Follow any stricter task-specific safety limits.
2. State one concrete technical question, its StayInk relevance, and the observation that would answer it. Define the target Kindle model, generation, firmware, and execution environment; use `UNKNOWN` for anything not established.
3. Search existing pinned checkouts and `docs/research/SOURCES.md` before adding sources. When adding a repository, pin a full commit SHA and record its URL, license, retrieval date, local path, relevance, and limitations. For mutable issues or web pages, record the URL and retrieval date without inventing a SHA.
4. Trace the narrow behavior across definitions, callers, event producers and consumers, state transitions, and platform branches. Cite exact files, symbols, and stable line locations where practical. Do not summarize an entire repository when a call path answers the question.
5. Compare sources without forcing agreement. Record contradictions, differing terminology, and generation-specific branches. Preserve each source's scope and explain whether a conclusion is directly supported or merely consistent with multiple sources.
6. Classify every important conclusion:
   - **FACT:** directly supported by cited source or observation, within the stated scope.
   - **INFERENCE:** reasoned beyond the direct evidence; list the supporting facts and assumptions.
   - **UNKNOWN:** missing, conflicting, or unverified; state what evidence would resolve it.
7. Attach to each important claim the source and revision, code or evidence location, model and firmware applicability, confidence, and StayInk relevance. Phrase negative results as “not found in the reviewed scope” unless the search was demonstrably exhaustive.
8. Treat Scribe applicability as **UNKNOWN** unless evidence demonstrates the behavior on the relevant Scribe generation and firmware. Event names, function names, comments, and reports such as “suspend,” “deep sleep,” or “hibernate” do not independently prove stock ownership, semantics, or low-power residency.
9. Inspect third-party licensing before reuse. Distinguish behavioral reference from copying source, assets, or documentation. Source access permits analysis, not reuse; do not copy GPL or AGPL code into StayInk, and mark unclear licensing as `UNKNOWN` until compatibility is verified.
10. Update the canonical records under `docs/research/` with concise evidence, not duplicated narrative. Finish with falsifiable hypotheses, their assumptions and risks, and the smallest safe next experiments with observable success and failure criteria. Keep device-changing work out of a research task unless it is explicitly authorized with rollback and recovery paths.

## Completion check

Before reporting completion, verify that every FACT has direct support, every applicability claim is scoped, source disagreements remain visible, licenses are recorded, and each proposed experiment can disprove the hypothesis it tests.
