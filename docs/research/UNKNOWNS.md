# Unknowns

## UNK-001 — Which stock Scribe component replaces the display?

- Classification: UNKNOWN
- Sources: SRC-003, SRC-004, SRC-008, SRC-009, OBS-001, OBS-002
- Model / firmware: all Scribe variants / all firmware
- Confidence: high that the question is correctly framed
- Relevance: a display-side intervention cannot be scoped safely without identifying the owner.
- Current evidence: FACT-013 maps six publishers to their target processes on firmware 5.19.5. Blanket's command line contains the literal `screensaver` argument, making blanket a stronger candidate participant by inference; KPPMainApp owns the `ScreenSaverListener` publisher. FACT-015 records a controlled KOReader transition but attributes no display operation to those stock candidates. Neither name, ownership, nor event order identifies the framebuffer writer or panel-refresh requester.
- Resolution criteria: a target-device timestamped trace that attributes the framebuffer write and panel-refresh request to a process/module without modifying it.

## UNK-002 — What proves genuine low-power suspend on Scribe?

- Classification: UNKNOWN
- Sources: SRC-004, SRC-008, OBS-001, OBS-002
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: preserving the image while merely keeping the device awake is invalid.
- Current evidence: FACT-014 provides direct kernel evidence of complete `mem` suspend/resume transactions on the target. FACT-015 adds a controlled transaction corroborated by a 76-to-77 suspend-success increment, zero observed failures, selected powerd events, and an 82.711-second kernel-reported interval. No reviewed evidence measures current draw or establishes the lowest hardware residency depth, and the controlled environment had KOReader active.
- Resolution criteria: correlate existing kernel suspend/resume evidence with non-invasive external power measurement and a stock control trial.

## UNK-003 — When does the replacement refresh occur relative to powerd events?

- Classification: UNKNOWN
- Sources: SRC-006, SRC-008, OBS-002
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: both candidates require a safe display-side window.
- Current evidence: FACT-015 establishes, for KOReader on the target firmware, about 60.009 seconds from `goingToScreenSaver` to the first observed readiness event and about 10.164 seconds from the final readiness event to kernel entry. Its independent video records the KOReader sleeping box 0.43 seconds after the physical button, but no shared marker synchronizes video with listener time. Stock replacement timing and refresh submission remain unobserved.
- Resolution criteria: run a separately reviewed stock-UI control with a shared visual/software synchronization marker, then attribute any refresh submission through read-only display/process telemetry if a safe observable is established.

## UNK-004 — Does the current panel image and framebuffer survive Scribe suspend/resume?

- Classification: UNKNOWN
- Sources: SRC-005, SRC-007, SRC-008
- Model / firmware: each Scribe generation and firmware
- Confidence: high
- Relevance: determines whether suppression alone is sufficient or restoration is necessary.
- Current evidence: E Ink is generally bistable; FBInk exposes separate memory and panel operations; long-sleep wake corruption is reported. No Scribe control experiment compares pixels, panel, and buffer across sleep.
- Resolution criteria: on a non-sensitive test image, compare visible output and framebuffer identity before sleep, during display transition, and after wake without initially writing either.

## UNK-005 — Can stock display replacement be bypassed without changing powerd progress?

- Classification: UNKNOWN
- Sources: SRC-004, SRC-008, SRC-009
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: this is the central StayInk question.
- Current evidence: application event handling and one modified-resource report show visible replacement can diverge from lock behavior, but neither measures suspend after a safe, display-only intervention.
- Resolution criteria: a reversible runtime test whose event timeline, kernel evidence, and external power match stock suspend while the original image remains visible.

## UNK-006 — What restores the normal UI on every wake path?

- Classification: UNKNOWN
- Sources: SRC-004, SRC-008, OBS-001, OBS-002
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: scheduled wakes, button wakes, cover wakes, passcode, and long-sleep wakes may require different restoration behavior.
- Current evidence: KOReader distinguishes suspend wake from screensaver exit and reports background wake/resuspend; recent Scribe users report wake-display corruption. FACT-015 orders kernel exit, `wakeupFromSuspend`, `outOfScreenSaver`, and `exitingScreenSaver` on one KOReader button cycle, but retained physical wake and restored-UI video times were unavailable. Phase 2A exposed other wake-related interfaces without mapping kernel labels to physical causes. The target had a passcode enabled and no magnetic cover fitted.
- Resolution criteria: control and candidate trials covering short/long sleep, button/cover wake, scheduled background wake when naturally observed, and normal UI verification.

## UNK-007 — What rootfs-free lifecycle can run a future mechanism?

- Classification: UNKNOWN
- Sources: SRC-001, SRC-002, SRC-003
- Model / firmware: KPM-supported Scribe / target firmware
- Confidence: high
- Relevance: KPM provides package and explicit launch hooks but no reviewed automatic power-event lifecycle.
- Current evidence: user payload and scriptlet locations are known; boot/background startup on Scribe is not.
- Resolution criteria: documented and device-confirmed lifecycle that uses existing jailbreak facilities, needs no rootfs modification, and has a fail-safe/uninstall path.

## UNK-008 — Which model, firmware, and UI variants change the path?

- Classification: UNKNOWN
- Sources: SRC-004, SRC-005, SRC-008, SRC-009, OBS-001, OBS-002
- Model / firmware: each community-mapped Scribe device set and variant / all relevant releases
- Confidence: high
- Relevance: support cannot be inferred across devices or releases.
- Current evidence: OBS-001 establishes one UI-identified Kindle Scribe on firmware 5.19.5 with passcode enabled, no Special Offers, no magnetic cover, Véra, KPM, and an active KOReader publisher. OBS-002 adds one controlled KOReader button trace on that same target. Its generation and physical model number remain unknown, no stock control is complete, and applicability to another model, firmware, UI, or observer transport remains unestablished.
- Resolution criteria: maintain a device/firmware matrix with repeated event, display, low-power, and wake observations for each claimed supported combination.

## UNK-009 — What are the remaining semantics of powerd's `g_is_screensaver_drawn` state?

- Classification: UNKNOWN
- Sources: SRC-003, SRC-004, SRC-006, OBS-006, OBS-007, OBS-008, OBS-009, OBS-010, OBS-011
- Model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5 for the observed log; other applicability UNKNOWN
- Confidence: high that powerd owns the observed set-to-one, reset, and consumer paths; high that the remaining semantic/presentation attribution gap is real
- Relevance: this state may expose the presentation-completion boundary StayInk needs, or it may instead be diagnostic or coupled to suspend progression.
- Current evidence: FACT-018 shows powerd logging the value 1 during a wake-side `SUSPENDED -> SCREEN SAVER` transition. FACT-019 identifies the embedded diagnostics, and FACT-021 establishes that powerd itself stores 1 after its `goingToScreenSaver` send helper returns, has two internal zero-store paths, and later reads the same global in a path tied to low-power frontlight handling. FACT-022 establishes that KPP's specifically named `ApplicationModule::screenSaverHandler()` subscribes to entry and exit events and performs engagement-metrics bookkeeping, not a demonstrated presentation acknowledgement. FACT-023 shows that the copied blanket launcher delegates loader parsing and event dispatch to `libblanket.so.1.0`; it exposes no direct completion report to powerd. The exact identifier remains absent from the reviewed pinned-source scope. Set/reset ownership is resolved to powerd for this binary; the field's intended meaning, transition coverage, lifetime, and any causal relationship to suspend remain unestablished.
- Resolution criteria: a separately reviewed read-only runtime correlation must show whether the field tracks only powerd transition bookkeeping/frontlight selection or any window visibility, image preparation, framebuffer/update completion, or suspend-readiness boundary.

## UNK-010 — Which downstream component submits the panel update after the screensaver plugin presents its window?

- Classification: UNKNOWN
- Sources: SRC-003, SRC-004, OBS-001, OBS-011, OBS-012, OBS-013, OBS-014, OBS-015, OBS-016
- Model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5 for the copied artifacts; other applicability UNKNOWN
- Confidence: high that name/path/loading and plugin content/window presentation are resolved; high that physical update submission and safe control semantics remain unresolved
- Relevance: the plugin callback is now the narrowest demonstrated stock content/window presentation boundary, but StayInk must know whether omitting that boundary leaves the panel unchanged without disrupting genuine suspend or wake.
- Current evidence: FACT-023 through FACT-027 establish the launcher, loader library, fixed module path, and canonical plugin. FACT-028 establishes that `screensaver.so.1.0` consumes `goingToScreenSaver`, prepares/renders default or book-cover content, creates a Cairo Xlib surface for `blanket_screensaver`, and calls blanket window bring-up/teardown helpers. FACT-029 narrows `blanket_image_window_bringup` to `XSelectInput`, `XMapRaised`, `XFlush`, and context bookkeeping, while showing that a subsequent X11 Expose callback can perform deferred rendering and a win-manager flash trigger. The plugin does not directly import a framebuffer/HWTCON/MXCFB or E-Ink update API. No reviewed call attributes the physical update request.
- Resolution criteria: a narrowly reviewed read-only observation must correlate the plugin window/layer transition or an existing display log with the visible replacement. Deeper HWTCON attribution is not required unless that correlation cannot answer the intervention question. Separately, any future intervention design requires proven runtime recovery and evidence that omitting only presentation preserves `readyToSuspend`, kernel `mem` suspend, normal wake restoration, and wake paths in scope.

## UNK-011 — Is there a coherent, non-patching runtime control for only the screensaver window presentation?

- Classification: UNKNOWN
- Sources: SRC-003, SRC-004, OBS-014, OBS-016, OBS-017, OBS-018, OBS-019, OBS-020, OBS-021, OBS-022, OBS-023
- Model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5 for the copied artifacts; other applicability UNKNOWN
- Confidence: high that the semantic boundary and the control gap are correctly separated
- Relevance: the first reversible StayInk experiment may not patch binaries and must preserve the stock lifecycle while changing only visible presentation.
- Current evidence: FACT-029 identifies a narrow X11 map/raise helper, but its caller ignores the helper return and marks the plugin presented. FACT-030 establishes static creation and retention of the screensaver window. SRC-003 generically documents a reversible Awesome `HIDE` title flag, while the focused winmgr/blanket inventories expose no documented narrow visibility setter. FACT-031 shows that post-map unmap is racy because the Expose callback paints before its deferred flash-trigger/render branches. SRC-004 demonstrates external X11 manipulation on some Kindle firmware but also reports `ligl` timeout and post-5.12.4 UI-softlock risks. FACT-032 through FACT-035 establish the available X query path, successful X access, loaded module, and exact runtime window. FACT-036 directly confirms XID addressability, reported blanket PID 4524, unmapped state, and correlated winmgr getter value 0. The read-only identity gate is complete; `HIDE` behavior, Awesome reaction, Expose/flash behavior while hidden, panel behavior, and wake safety remain unobserved.
- Resolution criteria: Phase 4A design is recorded in `docs/research/HIDE_METADATA_EXPERIMENT.md`; Phase 4B host implementation/audit and the unexecuted target procedures are recorded in `docs/research/GUARD_WATCHDOG_VALIDATION.md`. Before execution can be considered, audit the ARM no-write binary, prove exact restore on a disposable X server, and complete the target ordinary and detached-watchdog no-write runs. Only a separately authorized trial can resolve target `HIDE`, Awesome, Expose, panel, suspend-equivalence, and wake behavior.
