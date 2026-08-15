# Sources

Repository evidence is pinned to a full commit SHA. Web and issue evidence is mutable, so its retrieval date and revision metadata are recorded instead. Direct target-device observations use `OBS` identifiers and record the device, firmware, observation date, environment, and limitations without inventing a repository revision. All local checkouts are unmodified references under the gitignored `.research/repos/` directory.

## SRC-001 — KindleModding/KPM

- Repository: https://github.com/KindleModding/KPM
- Commit SHA: `ffa767fffadd731bd59f2bca8c83231f4fc0ab2d`
- License: GPL-3.0; root `LICENSE`
- Retrieved: 2026-08-14
- Local reference path: `.research/repos/KPM`
- Relevance: modern package storage, install/launch/uninstall hooks, and KPM's own runtime paths.
- Limitations: establishes KPM behavior, not stock Kindle power behavior. The README separately licenses its own text under CC0; that does not replace the repository's root license.

## SRC-002 — KindleModding/example_kpm_package

- Repository: https://github.com/KindleModding/example_kpm_package
- Commit SHA: `fa4d169716d3f3f07cd2ac215002014a9b600d4b`
- License: CC0-1.0; root `LICENSE`
- Retrieved: 2026-08-14
- Local reference path: `.research/repos/example_kpm_package`
- Relevance: concrete example of KPM hooks, package-relative execution, and scriptlet ownership.
- Limitations: example behavior is guidance, not a required service lifecycle.

## SRC-003 — KindleModding documentation

- Repository: https://github.com/KindleModding/kindlemodding.github.io
- Commit SHA: `cf4457c4808636dac5e06d2f2761cb4632efe66d`
- License: CC BY-NC 4.0; root `LICENSE`
- Retrieved: 2026-08-14
- Local reference path: `.research/repos/kindlemodding.github.io`
- Relevance: KPM packaging guidance, scriptlets, LIPC inventories, and model-scoped boot documentation.
- Limitations: several LIPC tables contain `TODO` descriptions and do not establish property semantics. The available boot-process page is scoped to Paperwhite 6, not Scribe.

## SRC-004 — KOReader

- Repository: https://github.com/koreader/koreader
- Commit SHA: `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`
- License: AGPL-3.0; root `COPYING`
- Retrieved: 2026-08-14
- Local reference path: `.research/repos/koreader`
- Relevance: Kindle-specific handling of powerd events, screensaver lifecycle, wake behavior, model detection, and Scribe display setup.
- Limitations: proves what KOReader observes or does. It does not by itself prove stock UI internals or genuine low-power residency.

## SRC-005 — FBInk

- Repository: https://github.com/KindleModding/FBInk
- Commit SHA: `13872c056d050f7079770a126ec37acd952afaba`
- License: GPL-3.0; root `LICENSE`
- Retrieved: 2026-08-14
- Local reference path: `.research/repos/FBInk`
- Relevance: framebuffer mapping, panel-refresh separation, waveform/update handling, and MediaTek Scribe device mappings.
- Limitations: an implementation reference, not proof of stock renderer behavior. No source is copied into StayInk.

## SRC-006 — KOReader base

- Repository: https://github.com/koreader/koreader-base
- Commit SHA: `809c16e24427fe4a722a56ea1e4f9b32372aac95`
- License: AGPL-3.0; root `COPYING`
- Retrieved: 2026-08-14
- Local reference path: `.research/repos/koreader/base`
- Relevance: the pinned KOReader submodule contains the Kindle LIPC event monitor and framebuffer bindings referenced by SRC-004.
- Limitations: only this required submodule was initialized. It inherits the same application-vs-stock limitation as KOReader.

## SRC-007 — E Ink technology overview

- Source: https://www.eink.com/tech/detail/Benefits
- Commit SHA: NOT APPLICABLE
- License: UNKNOWN
- Retrieved: 2026-08-14
- Local reference path: NOT CLONED
- Relevance: first-party description of E Ink display bistability.
- Limitations: general panel physics only; it does not establish Kindle Scribe framebuffer, controller, suspend, or resume behavior.

## SRC-008 — KOReader issue 15757 and attached log

- Repository: https://github.com/koreader/koreader
- Evidence URL: https://github.com/koreader/koreader/issues/15757
- Commit SHA: NOT APPLICABLE; issue evidence
- Revision: created 2026-07-29, last updated 2026-08-11 when retrieved
- License: UNKNOWN for issue text and attachments; repository source is AGPL-3.0
- Retrieved: 2026-08-14
- Local reference path: NOT STORED IN STAYINK
- Relevance: recent Scribe reports and a KOReader log containing powerd-derived event timing on firmware 5.19.4.
- Limitations: user-submitted, mutable, and captured while KOReader was active. Labels such as "deep sleep" and "hibernation" are not independent power measurements.

## SRC-009 — KOReader issue 13330

- Repository: https://github.com/koreader/koreader
- Evidence URL: https://github.com/koreader/koreader/issues/13330
- Commit SHA: NOT APPLICABLE; issue evidence
- Revision: created 2025-02-26, last updated 2025-06-09
- License: UNKNOWN for issue text and attachments; repository source is AGPL-3.0
- Retrieved: 2026-08-14
- Local reference path: NOT CLONED
- Relevance: a Scribe 2022 report where lock continued without sleep-image replacement after stock screensaver resources had been modified.
- Limitations: the device root filesystem had been changed, genuine suspend was not measured, and the result is unsafe to reproduce as a StayInk experiment.

## SRC-010 — KOReader issue 13091

- Repository: https://github.com/koreader/koreader
- Evidence URL: https://github.com/koreader/koreader/issues/13091
- Commit SHA: NOT APPLICABLE; issue evidence
- Revision: created 2025-01-17, last updated 2025-09-21
- License: UNKNOWN for issue text and attachments; repository source is AGPL-3.0
- Retrieved: 2026-08-14
- Local reference path: NOT CLONED
- Relevance: a Scribe cover-wake report resolved by changing KOReader's own “ignore cover” setting.
- Limitations: firmware is not stated, the reported KOReader version appears malformed, and the issue does not establish stock hall-sensor behavior.

## OBS-001 — Phase 2A target Kindle Scribe reconnaissance

- Source: direct, manually captured target-device observations supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: jailbroken with Véra and KPM; passcode enabled; no Special Offers; no magnetic cover; KOReader's `com.github.koreader.kindlepowerd` publisher was active
- Evidence location: approved read-only commands in `docs/research/DEVICE_PROBE.md`; redacted results recorded in FACT-013, FACT-014, and the Phase 2A closure section of `docs/research/SCRIBE.md`
- License: NOT APPLICABLE to owner-supplied observations
- Relevance: establishes the actual target environment, candidate D-Bus publisher owners, and available kernel suspend evidence before Phase 2B design.
- Limitations: point-in-time snapshots from one Scribe on one firmware; the Scribe generation remains unknown; raw output is not committed because it included a device serial; the active KOReader publisher means this was not a pure stock baseline; no controlled display/event trace or external power measurement was performed.

## OBS-002 — Phase 2B Baseline A controlled KOReader trace

- Source: direct, manually captured target-device commands and external phone-video observations supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM with KOReader active and its `com.github.koreader.kindlepowerd` publisher present; SSH-hosted read-only listener; not a pure-stock baseline
- Evidence location: accepted execution record in `docs/research/POWER_TRACE.md`; distilled as FACT-015
- License: NOT APPLICABLE to owner-supplied observations
- Relevance: correlates selected powerd events with an independently recorded kernel `mem` suspend/resume transaction and records the separate KOReader visual transition.
- Limitations: the phone video had no shared synchronization marker with the listener clock; physical wake/restored-UI times were unavailable; the active KOReader UI/publisher and SSH observer may affect timing; no framebuffer access or electrical measurement was performed; the listener's local wall-clock-like timestamp behavior is scoped to this target/trial and is not generalized to other versions.

## OBS-003 — Phase 2B Baseline B revision 1 observer gate failure

- Source: direct target-device output supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM with KOReader and its SSH transport active during observer setup; Baseline B had not started
- Evidence location: revision 1 gate result in `docs/research/POWER_TRACE.md`
- License: NOT APPLICABLE to owner-supplied observations
- Relevance: establishes that `mkfifo` at `/mnt/us/stayink-observation/baseline-b/events.pipe` returned `Operation not permitted`, invalidating the FIFO-based local observer before any stock Notebook trial.
- Limitations: establishes rejection of that exact FIFO creation on this target and environment only; it does not characterize all filesystems or FIFO implementations. The observer exited before creating listener/timer PID files, so no powerd event stream or Baseline B suspend cycle was captured.

## OBS-004 — Phase 2B Baseline B revision 2 stock-Notebook attempt

- Source: direct target-device output and external phone-video observations supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; observer launched through KOReader SSH and reported stock Kindle Notebook foreground after ordinary KOReader exit; not a proven pure-stock boot
- Evidence location: revision 2 trial result in `docs/research/POWER_TRACE.md`; distilled as FACT-016
- License: NOT APPLICABLE to owner-supplied observations
- Relevance: validates detached local event capture and records a stock-Notebook visual replacement plus two powerd screensaver episodes without continuous SSH.
- Limitations: no kernel `mem` suspend occurred during the observer window; the 360-second timer ended during repeated `readyToSuspend 10` events; no shared marker aligns the phone-video clock with either software episode; the cause of the first episode and the reason for the second episode's defer state are UNKNOWN; the pre/post suspend-counter interval included an earlier uncontrolled kernel transaction.

## OBS-005 — Phase 2B Baseline B revision 2 attempt 2

- Source: direct target-device output supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; observer launched through KOReader SSH and reported stock Kindle Notebook foreground after ordinary KOReader exit; not a proven pure-stock boot
- Evidence location: revision 2 attempt 2 record in `docs/research/POWER_TRACE.md`; distilled as FACT-017
- License: NOT APPLICABLE to owner-supplied observations
- Relevance: captures selected powerd events around a genuine kernel `mem` transaction without continuous SSH and empirically characterizes the one-shot timer across suspend.
- Limitations: the execution occurred after the prior decision gate said not to repeat revision 2 unchanged; no phone-video or shared synchronization marker was supplied for this attempt; no immediately preceding suspend-statistics snapshot was supplied; event payload definitions, renderer ownership, panel-refresh ownership, framebuffer retention, electrical low-power depth, and pure-stock applicability remain UNKNOWN.
