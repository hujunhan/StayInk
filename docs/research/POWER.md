# Kindle Power Path

## Evidence-backed model

The internal stock state machine remains UNKNOWN. The smallest model supported by current evidence is an observable-event model:

```text
sleep request
  -> goingToScreenSaver(source)
  -> [display replacement timing and actor UNKNOWN]
  -> readyToSuspend(delay), possibly repeated
  -> [actual power state and depth not independently measured]
  -> wakeupFromSuspend(elapsed)
  -> outOfScreenSaver(source), for a user-visible exit
```

This is not asserted as the implementation. It is the event order seen by KOReader in selected cases.

## Direct observations

### Powerd is the observable event source

Classification: FACT — FACT-001, FACT-002, FACT-004

KOReader's Kindle input layer subscribes to five distinct `com.lab126.powerd` events relevant here. Its UI layer separately handles screen-saver entry/exit and suspend readiness/wake. When KOReader asks for the power-button behavior, it writes `com.lab126.powerd powerButton=1`.

The KindleModding LIPC inventory at SRC-003 also lists readable `state` and writable properties named `preventScreenSaver`, `deferSuspend`, and `abortSuspend`, but their descriptions are `TODO`. Their names are not evidence of safe semantics, and none is a candidate until observed on Scribe.

### Button and cover sources are distinguishable in KOReader

Classification: FACT about KOReader; Scribe stock meaning UNKNOWN

`frontend/device/kindle/device.lua` maps values seen with screen-saver events to button suspend/wake (`2`/`1`) and hall suspend/wake (`4`/`6`). The 5.19.4 Scribe attachment in SRC-008 contains both button and hall-labelled sequences. KOReader notes that programmatic and physical button requests are not distinguishable at this layer.

### A recent Scribe log separates the stages in time

Classification: FACT — FACT-003

In one 5.19.4 sequence, screen-saver entry precedes the first readiness callback by about 60 seconds. Readiness callbacks carry countdown-like values, a wake event later reports elapsed time, and an `outOfScreenSaver` event follows. Other sequences contain a wake while powerd still reports `screenSaver`, after which another readiness cycle starts.

The event called `wakeupFromSuspend` is evidence that powerd emitted that event. It is not, by itself, proof of current draw, kernel suspend depth, or the time spent in the lowest available state.

### The target kernel records `mem` suspend/resume transactions

Classification: FACT — FACT-014

On the observed Scribe running firmware 5.19.5, existing kernel-ring-buffer sequences contain `PM: suspend entry`, preparation and system suspension for `mem`, completed device/noirq phases, a platform-reported suspended duration, completed resume phases, and `PM: suspend exit`. This is direct evidence of kernel system suspend/resume handling and is stronger than screensaver state or a powerd event name.

The observation does not measure current draw or identify the lowest hardware residency depth. KOReader's `com.github.koreader.kindlepowerd` publisher was active during Phase 2A, so the environment is not a pure stock baseline. A later controlled trial still needs an external power measurement if StayInk is to claim genuine low-power behavior equivalent to control.

## Current synthesis

### The display phase and suspend-readiness phase are observably distinct

Classification: INFERENCE

Because KOReader receives different events and performs its own screen-saver work on `goingToScreenSaver`, the project should investigate the interval before `readyToSuspend` rather than altering the sleep request itself. This does not show when the stock renderer draws, whether it blocks readiness, or whether the events are contractual.

The wake-while-`screenSaver` sequences are consistent with a background wake followed by resuspend. Their cause is not directly demonstrated, so this remains an inference and a test case rather than a fact about every wake.

### The safest conceptual boundary is display-side, not powerd-side

Classification: INFERENCE

StayInk's invariant argues for leaving the `powerButton` request, readiness sequence, and resume path untouched. A mechanism that changes `preventScreenSaver`, `deferSuspend`, or `abortSuspend` risks changing power state and is therefore excluded until its semantics and low-power result are measured.

### Where replacement can safely be avoided

Classification: UNKNOWN

Current evidence narrows the search to a display-side action associated with screen-saver entry and completed before real suspend. It does not yet identify a safe hook or prove that suppressing the renderer leaves powerd unaffected on any Scribe firmware.

The implementation-blocking gaps are tracked as UNK-001 through UNK-006 rather than duplicated here.
