# Unknowns

## UNK-001 — Which stock Scribe component replaces the display?

- Classification: UNKNOWN
- Sources: SRC-003, SRC-004, SRC-008, SRC-009
- Model / firmware: all Scribe variants / all firmware
- Confidence: high that the question is correctly framed
- Relevance: a display-side intervention cannot be scoped safely without identifying the owner.
- Current evidence: KOReader refers to blanket, pillow, and the window manager in different UI modes; KindleModding inventories their LIPC interfaces; none is stock Scribe source or a process trace of the replacement.
- Resolution criteria: a target-device timestamped trace that attributes the framebuffer write and panel-refresh request to a process/module without modifying it.

## UNK-002 — What proves genuine low-power suspend on Scribe?

- Classification: UNKNOWN
- Sources: SRC-004, SRC-008
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: preserving the image while merely keeping the device awake is invalid.
- Current evidence: event names and elapsed times suggest suspend, but no reviewed evidence measures power, kernel state, or residency depth.
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
- Sources: SRC-004, SRC-008
- Model / firmware: Scribe / target firmware
- Confidence: high
- Relevance: scheduled wakes, button wakes, cover wakes, passcode, and long-sleep wakes may require different restoration behavior.
- Current evidence: KOReader distinguishes suspend wake from screensaver exit and reports background wake/resuspend; recent Scribe users report wake-display corruption.
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
- Sources: SRC-004, SRC-005, SRC-008, SRC-009
- Model / firmware: Scribe generations 1–3 and variants / all relevant releases
- Confidence: high
- Relevance: support cannot be inferred across devices or releases.
- Current evidence: community model tables differ by generation; issue evidence spans 5.17.2–5.19.4 but remains KOReader-specific; stock passcode and Special Offers paths are not characterized.
- Resolution criteria: maintain a device/firmware matrix with repeated event, display, low-power, and wake observations for each claimed supported combination.
