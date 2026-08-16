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

## OBS-006 — Phase 3A existing-log candidate search

- Source: direct target-device output supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; KOReader has been present; the captured sequence is not a proven pure-stock boot or the accepted Baseline B attempt-2 interval
- Evidence location: distilled in `docs/research/DISPLAY_PATH_ATTRIBUTION.md` and FACT-018; raw output is not committed
- License: NOT APPLICABLE to owner-supplied observations
- Relevance: directly establishes a wake-side KPPMainApp response to screensaver-stop state on the target and records a powerd `g_is_screensaver_drawn` log label without attributing rendering.
- Limitations: the supplied matching lines cover a later wake-side sequence around local-like `260814:231011`, not the accepted Baseline B entry/replacement interval; they contain no `goingToScreenSaver` record or candidate action that creates, selects, shows, or layers a screensaver. No matching blanket, winmgr/Awesome, pillow, or blankwindow line was supplied, but this is only a negative result for the retained log and search expression. The raw output contained notebook/content identifiers and is deliberately not retained.

## OBS-007 — Phase 3B target powerd string inspection

- Source: direct target-device output supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; awake, read-only static inspection of `/usr/bin/powerd`; KOReader has been present, so this is not a proven pure-stock boot
- Evidence location: distilled in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md` and FACT-019; the full proprietary string output is not committed
- License: target stock binary license UNKNOWN; short identifiers and diagnostic labels are retained only as behavioral evidence
- Relevance: establishes that the target powerd binary embeds the exact field diagnostic, a separate reset-to-false diagnostic, blanket configuration-key names, and related screensaver/readiness event names.
- Limitations: `strings` output proves only that printable sequences occur in the executable. Context order is file order, not a call graph; it does not identify cross-references, assignments, runtime execution, the set-to-true path, or a blanket callback. No other candidate binary was inspected in this observation.

## OBS-008 — Phase 3B cross-candidate string comparison

- Source: direct target-device output supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; awake, read-only static inspection of `/usr/sbin/blanket`, `/app/bin/KPPMainAppV2`, and `/usr/bin/awesome`; not a proven pure-stock boot
- Evidence location: distilled in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md` and FACT-020; the full proprietary string output is not committed
- License: target stock binary licenses UNKNOWN; only short identifiers are retained as behavioral evidence
- Relevance: identifies a KPP application-module screensaver handler and readiness-related strings while finding no exact selected match in the blanket or Awesome binaries.
- Limitations: filtered `strings` output is not a call graph or runtime trace. No match in blanket/Awesome is scoped to these files and this pattern; stripping, indirection, shared libraries, or different terminology remain possible. KPP's thumbnail/TOC draw-completion strings explicitly name other rendering work and do not establish sleep-screen completion.

## OBS-009 — Phase 3B symbol-tool availability failure

- Source: direct target-device output supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; target shell reported `readelf` unavailable
- Evidence location: Stage 3 result in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md`
- License: NOT APPLICABLE
- Relevance: closes the approved symbol-table gate without expanding the target toolset or substituting a broader inspector.
- Limitations: establishes only that `readelf` was not available to that target shell through its current command path. No symbol table was read; it does not establish that the binaries lack symbols or that another unreviewed tool is absent.

## OBS-010 — Phase 3B local static analysis of copied target binaries

- Source: owner-supplied copies of the target `/usr/bin/powerd` and `/app/bin/KPPMainAppV2`, analyzed locally without execution
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: copied from the UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: local read-only ELF metadata, dynamic-symbol, disassembly, literal-reference, and call-site inspection; no Kindle interaction
- Evidence artifacts: `docs/kindle_file/powerd` SHA-256 `72f3c3a0d5141202df98506a750070c7f033f42b1711122aa7060770cedfce3e`; `docs/kindle_file/KPPMainAppV2` SHA-256 `03a956a168347ac84b4cfd0a92620c37ea7b244b0c2a6ae74c2334b069093964`
- Evidence location: distilled in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md`, FACT-021, and FACT-022; binaries and full proprietary disassembly/string output are not research-document content and must not be committed as source
- License: target stock binary licenses UNKNOWN; analysis records only the minimum identifiers, instruction relationships, and hashes needed as behavioral evidence
- Relevance: identifies powerd's actual stores to the `g_is_screensaver_drawn` global and resolves KPP's named application-module handler as a powerd-event metrics consumer rather than a demonstrated presentation-completion callback.
- Limitations: both ELF files are stripped and reference separate `.dbg` files that were not supplied. Static control flow does not prove a path executed in a particular trial, formal LIPC delivery semantics, field lifetime across every transition, framebuffer/panel ownership, or applicability beyond this one firmware image.

## OBS-011 — Phase 3 local static analysis of copied target blanket launcher

- Source: owner-supplied copy of the target `/usr/sbin/blanket`, analyzed locally without execution
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: copied from the UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: local read-only ELF metadata, dynamic-symbol, printable-string, and ARM Thumb call-site inspection; no Kindle interaction
- Evidence artifact: `docs/kindle_file/blanket` SHA-256 `46471158030d1b961d60fd9be261532ef49801d5c5fa8a671b90b2d1cdb3c744`
- Evidence location: distilled in `docs/research/DISPLAY_PATH_ATTRIBUTION.md`, `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md`, and FACT-023; the binary and full proprietary disassembly/string output must not be committed as source
- License: target stock binary license UNKNOWN; analysis records only the minimum identifiers, instruction relationships, and hash needed as behavioral evidence
- Relevance: establishes that the target blanket executable is a loader-driven X11 event-loop launcher which delegates initialization, loader parsing, event queuing, and dispatch to `libblanket.so.1.0`; it also resolves the executable's `-t` option description.
- Limitations: the executable is stripped and references an unsupplied `blanket.dbg`. The linked `libblanket.so.1.0`, loader modules, configuration, and other shared objects were not supplied, so this observation cannot identify the screensaver module's event subscriptions, window/content actions, framebuffer access, or E-Ink update submission. Static linkage and call sites do not prove which loaders executed in a particular trial or which shared object ultimately handled an event.

## OBS-012 — Phase 3C focused blanket loader-boundary analysis

- Source: owner-supplied copy of the target `/usr/sbin/blanket` plus the existing pinned SRC-003 and SRC-004 references; analyzed locally without execution
- Repository / commit: target binary NOT APPLICABLE; SRC-003 `cf4457c4808636dac5e06d2f2761cb4632efe66d`; SRC-004 `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`
- Observed: 2026-08-15 UTC
- Device / firmware: binary copied from the UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5; community-source applicability otherwise limited as recorded in SRC-003/SRC-004
- Environment: local read-only ELF interpreter/dynamic-section, relocation, imported-symbol, string, and narrow ARM Thumb caller inspection; focused pinned-source search; no Kindle interaction
- Evidence artifact: `docs/kindle_file/blanket` SHA-256 `46471158030d1b961d60fd9be261532ef49801d5c5fa8a671b90b2d1cdb3c744`
- Evidence location: distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md` and FACT-024; the binary and full proprietary disassembly/string output must not be committed as source
- License: target stock binary license UNKNOWN; SRC-003 is CC BY-NC 4.0; SRC-004 is AGPL-3.0 and used only as behavioral reference
- Relevance: establishes that loader-name-to-implementation resolution is wholly beyond the launcher's imported `blanket_event_parse_loader()` call boundary and identifies the target-resolved `libblanket.so.1.0` as the exact next artifact.
- Limitations: at OBS-012, `libblanket.so.1.0` was not available locally, and ELF dynamic metadata could not bind each unversioned undefined symbol to a particular `DT_NEEDED` object. No reviewed pinned source implements the parser. OBS-014 later resolves the parser/path/ABI questions; plugin existence and display operations remain unresolved.

## OBS-013 — Target path resolution for `libblanket.so.1.0`

- Source: direct target-device pathname metadata supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; exact-path `ls -l` and `readlink -f` result; no broad filesystem enumeration
- Evidence location: distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md` and FACT-025
- License: NOT APPLICABLE to pathname metadata; target stock library license remains UNKNOWN
- Relevance: closes the Phase 3C pathname gate and identifies `/usr/lib/libblanket.so.1.0` as the single next artifact for offline parser analysis.
- Limitations: pathname, type bits, ownership, size, and self-resolution do not establish the library hash, ELF identity, SONAME, exports, parser implementation, runtime mapping, or whether another screensaver plugin exists.

## OBS-014 — Phase 3C local static analysis of copied `libblanket.so.1.0`

- Source: owner-supplied copy of the target `/usr/lib/libblanket.so.1.0`, analyzed locally without execution
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: copied from the UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: local read-only ELF metadata, dynamic-symbol, printable-string, and narrow ARM Thumb parser/loader call-path inspection; no Kindle interaction
- Evidence artifact: `docs/kindle_file/libblanket.so.1.0` SHA-256 `30fb3dd09ee73ead89058d44a389a73b79c40d4192428e222c7fda9357459eef`
- Evidence location: parser `0x73a4`–`0x74f7`, loader-event handler `0x6efc`–`0x70bf`, path resolver `0xa98c`–`0xab53`, six-symbol resolver `0xb500`–`0xb64f`, and load path `0xbc78`–`0xbf93`, distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md` and FACT-026; the proprietary library and full disassembly/string output must not be committed as source
- License: target stock library license UNKNOWN; analysis records only the minimum identifiers, instruction relationships, path convention, ABI names, and hash needed as behavioral evidence
- Relevance: resolves the `screensaver` loader as a dynamically loaded `/usr/lib/blanket/screensaver.so` candidate, identifies its six-symbol module ABI, and establishes that the common library offers X11/window and fbdev/image capabilities without attributing their use to the plugin.
- Limitations: at OBS-014, the exact `screensaver.so` target pathname had not yet been observed and the plugin was not supplied. OBS-015 later confirms the symlink and canonical path. Static common-library capability does not prove that this plugin calls a window, framebuffer, display-update, or panel-refresh function or that a particular path ran during the observed replacement.

## OBS-015 — Target path resolution for blanket `screensaver.so`

- Source: direct target-device pathname metadata supplied by the device owner
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: Véra/KPM; exact-path `ls -l` and `readlink -f` result; no broad filesystem enumeration
- Evidence location: distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md` and FACT-027
- License: NOT APPLICABLE to pathname metadata; target stock plugin license remains UNKNOWN
- Relevance: closes the plugin-existence/path gate and identifies `/usr/lib/blanket/screensaver.so.1.0` as the single next proprietary artifact for offline callback and presentation-path analysis.
- Limitations: symlink name, target, owner/group, and timestamp do not establish file size, content hash, ELF identity, exports, dependencies, runtime loading, presentation behavior, framebuffer access, or panel-refresh ownership.

## OBS-016 — Phase 3C local static analysis of copied `screensaver.so.1.0`

- Source: owner-supplied copy of the target `/usr/lib/blanket/screensaver.so.1.0`, analyzed locally without execution
- Repository / commit: NOT APPLICABLE
- Observed: 2026-08-15 UTC
- Device / firmware: copied from the UI-identified Kindle Scribe, generation and physical model number UNKNOWN / 5.19.5
- Environment: local read-only ELF metadata, dynamic-symbol, callback-table, printable-string, and narrow ARM Thumb presentation-path inspection; no Kindle interaction
- Evidence artifact: `docs/kindle_file/screensaver.so.1.0`, 34,660 bytes, SHA-256 `864a9987ea0c556f7e837d7433c265f862845973444cc74aeba3674ed77acf4e`
- Evidence location: module ABI and presentation path distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md` and FACT-028; the proprietary plugin and full disassembly/string output must not be committed as source
- License: target stock plugin license UNKNOWN; analysis records only the minimum identifiers, callback relationships, imported interfaces, and hash needed as behavioral evidence
- Relevance: establishes that the blanket `screensaver` plugin consumes screensaver lifecycle events, prepares/renders screensaver content, and controls an X11 screensaver window, separating that presentation ownership from still-unattributed framebuffer/panel-update submission.
- Limitations: the ELF is stripped and references an unsupplied `screensaver.so.1.0.dbg`. Static control flow does not prove which branch ran in a particular trial, the formal semantics of every event payload, who submits the E-Ink update after an X11 map/repaint, or whether suppressing the presentation callback preserves readiness and wake behavior.
