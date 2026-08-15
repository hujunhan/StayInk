# Unknowns

## UNK-001 — Which stock Scribe component replaces the display?

- Classification: UNKNOWN
- Sources: SRC-003, SRC-004, SRC-008, SRC-009, OBS-001
- Model / firmware: all Scribe variants / all firmware
- Confidence: high that the question is correctly framed
- Relevance: a display-side intervention cannot be scoped safely without identifying the owner.
- Current evidence: FACT-013 maps six publishers to their target processes on firmware 5.19.5. Blanket's command line contains the literal `screensaver` argument, making blanket a stronger candidate participant by inference; KPPMainApp owns the `ScreenSaverListener` publisher. Neither name nor ownership attributes the framebuffer write or panel-refresh request. KOReader's active publisher also means the observed environment was not a pure stock baseline.
- Resolution criteria: a target-device timestamped trace that attributes the framebuffer write and panel-refresh request to a process/module without modifying it.

## UNK-002 — What proves genuine low-power suspend on Scribe?

- Classification: UNKNOWN
- Sources: SRC-004, SRC-008, OBS-001
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: preserving the image while merely keeping the device awake is invalid.
- Current evidence: FACT-014 provides direct kernel evidence of complete `mem` suspend/resume transactions on the target. No reviewed evidence measures current draw or establishes the lowest hardware residency depth, and the observed environment included an active KOReader publisher.
- Resolution criteria: correlate existing kernel suspend/resume evidence with non-invasive external power measurement and a stock control trial.

## UNK-003 — When does the replacement refresh occur relative to powerd events?

- Classification: UNKNOWN
- Sources: SRC-006, SRC-008
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: both candidates require a safe display-side window.
- Current evidence: one Scribe log has about a minute between screen-saver entry and initial readiness, but it does not timestamp the visible replacement or its refresh submission.
- Resolution criteria: externally timestamp visible change while subscribing to events and tracing only already-available display/process telemetry.

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
- Sources: SRC-004, SRC-008, OBS-001
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: scheduled wakes, button wakes, cover wakes, passcode, and long-sleep wakes may require different restoration behavior.
- Current evidence: KOReader distinguishes suspend wake from screensaver exit and reports background wake/resuspend; recent Scribe users report wake-display corruption. Phase 2A exposed power-key, hall, RTC/alarm, WLAN, USB, touch, and stylus-related interfaces on the target, but did not map kernel wake labels to physical causes or test restoration through any path. The target had a passcode enabled and no magnetic cover fitted.
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
- Sources: SRC-004, SRC-005, SRC-008, SRC-009, OBS-001
- Model / firmware: each community-mapped Scribe device set and variant / all relevant releases
- Confidence: high
- Relevance: support cannot be inferred across devices or releases.
- Current evidence: OBS-001 establishes one UI-identified Kindle Scribe on firmware 5.19.5 with passcode enabled, no Special Offers, no magnetic cover, Véra, KPM, and an active KOReader publisher. Its generation and physical model number remain unknown. Community model tables differ by generation, and earlier issue evidence remains KOReader-specific.
- Resolution criteria: maintain a device/firmware matrix with repeated event, display, low-power, and wake observations for each claimed supported combination.
