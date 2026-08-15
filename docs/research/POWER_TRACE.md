# Kindle Scribe Phase 2B Power-Transition Trace Specification

Status: COMPLETE; Baseline A accepted as successful software/kernel evidence
Designed: 2026-08-14; executed: 2026-08-15 UTC
Target: UI-identified Kindle Scribe, generation **UNKNOWN**, firmware 5.19.5
Environment: Véra/KPM with KOReader's `com.github.koreader.kindlepowerd` publisher active
Trial label: **Baseline A — KOReader / current observed environment**

## Question and boundary

The concrete Phase 2B question is: **Where does the visible sleep-screen replacement occur relative to a human power-button request, selected `com.lab126.powerd` events, kernel `mem` suspend/resume, and visible restoration of the normal UI?** The answer is relevant because a future StayInk intervention must preserve the displayed image without delaying genuine suspend or breaking wake.

This experiment observes one normal manual power-button sleep/wake cycle. It does not identify the framebuffer writer, read or write framebuffer pixels, request suspend through software, inject input, change a LIPC property, enable tracing, control a service, alter logs, or modify a device file. No executable probe is justified: the physical actions, video framing, passcode privacy, observer behavior, and stop decisions require a human.

Important evidence labels:

- **FACT:** Phase 2A demonstrated that this target records complete kernel `mem` suspend/resume transactions in its existing kernel ring buffer and exposes readable `/sys/kernel/debug/suspend_stats`.
- **FACT:** The current environment includes KOReader's publisher, owned by `/mnt/us/koreader/luajit` running `reader.lua` (FACT-013). This is not a proven pure-stock baseline.
- **INFERENCE:** Correlating three independent timelines can locate the visible replacement within an approximate software/kernel interval.
- **UNKNOWN:** Which component renders, writes, or submits the replacement; whether the physical panel update completes at the same instant as any software call; electrical power draw and hardware residency depth.

## Safety classification

Only the exact commands classified **READ_ONLY** in this document are proposed. Here, READ_ONLY means the command has no property-set, file-write, log-clear, trace-enable, suspend, input, framebuffer, or service-control operation. It can still create observer effects: the shell and listener consume CPU and memory, create a transient D-Bus subscription, may generate ordinary session/audit records, and—when carried over SSH—keep a network connection present.

No command below is **UNCERTAIN** or **STATE_CHANGING**. If the target rejects an option, requests configuration, or behaves differently from the reviewed form, record the error and stop; do not improvise a replacement.

## Evidence for the narrow LIPC observer

### Reviewed source behavior

**FACT:** In SRC-006, KOReader base at commit `809c16e24427fe4a722a56ea1e4f9b32372aac95`, `input/input-kindle.h` lines 143–160 launches `lipc-wait-event -m -s 0 com.lab126.powerd <event-list>` as a child opened for reading, describes the invocation as listening forever for multiple events, and reads each emitted line from stdout. Lines 161–199 separately parse `goingToScreenSaver`, `outOfScreenSaver`, `exitingScreenSaver`, `wakeupFromSuspend`, and `readyToSuspend` with their payloads. This establishes KOReader's subscription use, not stock Scribe event semantics.

**FACT:** In SRC-004 at commit `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`, `frontend/device/kindle/device.lua` lines 218–235 show `-t` on the same command form and example output prefixed with timestamps such as `[00:00:04.675699]`. That source example does not establish whether the prefix is elapsed time, local wall time, or another clock across Kindle versions.

**FACT:** In SRC-003 at commit `cf4457c4808636dac5e06d2f2761cb4632efe66d`, `content/kindle-apps-and-services/_index.md` lines 52–64 describes LIPC events as D-Bus signals and D-Bus signals as broadcasts. A second subscriber therefore observes broadcasts; the reviewed model does not treat it as consuming an event from another listener.

**FACT:** SRC-006 `input/input-kindle.h` lines 28–34 says `lipc-wait-event` handles `SIGINT` properly and uses that signal for listener cleanup. For the manual foreground invocation, one terminal `Ctrl-C` is the reviewed exit action. It targets only the newly started observer, not powerd, KOReader, or another service. Do not use `kill`, `killall`, or a service control if it fails to exit.

### Exact listener and arguments

```sh
lipc-wait-event -m -s 0 -t com.lab126.powerd goingToScreenSaver,outOfScreenSaver,exitingScreenSaver,readyToSuspend,wakeupFromSuspend
```

| Argument | Meaning in the reviewed use |
| --- | --- |
| `lipc-wait-event` | Starts the installed LIPC event-waiting client. Phase 2A confirmed its path on the target. |
| `-m` | Multiple-event mode; continue after an event rather than returning after the first. |
| `-s 0` | No finite wait timeout in the pinned invocation; together with `-m`, KOReader describes this form as listening forever. |
| `-t` | Prefix output with a timestamp. On this target it produced local wall-clock-like values aligned with UTC−07:00 relative to the adjacent device UTC markers. That observed relation is target- and trial-scoped; the option's semantics are not assumed universal across versions. |
| `com.lab126.powerd` | Restricts the publisher to powerd. |
| comma-separated final argument | Restricts observation to exactly the five named candidate events. It is not a wildcard and does not discover other interfaces. |

**SAFETY CLASSIFICATION:** READ_ONLY.

**Why it is believed read-only:** The pinned implementation uses this command as a stdout-reading event subscription; the event model is broadcast signals, not property writes; and no argument requests a property set, event send, suspend, or persistent configuration. The subscription and D-Bus connection exist only for the observer process lifetime. No evidence in the reviewed form indicates mutation of persistent or power-management state.

**Availability and observer risk:** The exact options executed successfully on firmware 5.19.5 during Baseline A. The listener adds a process and D-Bus connection and emits output around transitions. If transported over SSH, that output and the connection can keep Wi-Fi/network activity present, change suspend timing, or cause a wake. The command is therefore suitable for ordering in this observed environment, not for power-consumption measurement.

## Three evidence timelines

### 1. Physical / visual timeline

Use one phone video with a visible running-time counter. Before connecting, record the trial label, firmware 5.19.5, battery/charging state, Wi-Fi state, portrait orientation, no magnetic cover, passcode state, and whether KOReader's publisher is present. Use a non-sensitive, visually distinctive starting page. Baseline A used KOReader; any later stock control should use a stock Settings page and receive its own review.

Frame the Kindle, the operator's power-button hand, and enough of the development-machine terminal to see the synchronization marker and event lines. If one fixed frame cannot show all three, prioritize the entire Kindle panel and button; briefly pan to the terminal at the pre/post markers without stopping the recording. Record:

1. the unchanged starting page for at least five seconds;
2. the physical sleep-button press;
3. the first visible E-Ink activity, the replacement's completion, and the stable sleep image;
4. the physical wake-button press;
5. the first visible wake activity and stable passcode/normal UI;
6. the original Settings page after unlock, if this can be done without exposing the passcode.

Do not share footage containing the passcode, account details, serial number, network identifiers, or personal content. If unlock is needed, obscure the passcode region and operator input while keeping the video continuous; otherwise treat the stable passcode screen as restoration of the normal protected wake UI and record that the post-unlock page was not observed.

The visual timestamps mark physical phenomena. They do not identify when framebuffer memory changed or when a panel-refresh ioctl was submitted.

### 2. Amazon power-management event timeline

The narrow listener records only the five candidate powerd broadcasts. Do not assume every event exists, occurs once, or has the payload meanings inferred by KOReader. Preserve raw names, payloads, repeats, and timestamp prefixes exactly.

The event names prove only that powerd published named events. In particular, `readyToSuspend` does not prove kernel suspend entry, and `wakeupFromSuspend` does not prove a particular hardware residency depth.

### 3. Kernel suspend/resume timeline

Take pre/post snapshots of the already-readable suspend statistics and a narrowly filtered view of existing kernel PM messages. The post-snapshot should expose new `PM: suspend entry`, `mem` preparation/suspension, suspended-duration, wake, resume, and `PM: suspend exit` lines if the controlled cycle reached the same kernel path observed in Phase 2A.

`/sys/power/wakeup_count` is deliberately not part of the minimum command set. Its value can change for wakeup events without uniquely proving a completed suspend, and it cannot order the visible replacement against kernel entry. It may be reconsidered only if target-specific semantics make it diagnostic; it must not be used as a suspend counter here.

## Synchronization method

1. Start the phone recording before the control's first `date` command.
2. Keep the terminal marker and Kindle in the same video frame when practical. Read the marker aloud as it appears.
3. The device UTC `date` output anchors the phone video to wall time within human-scale precision.
4. The listener's `-t` prefix must be interpreted from target observations rather than assumed to be elapsed time. In Baseline A it behaved like local wall time and aligned at UTC−07:00 with the adjacent device UTC and kernel timestamps.
5. After wake, the filtered kernel log supplies kernel-order timestamps and embedded UTC on `PM: suspend entry` and `PM: suspend exit` plus a platform-reported suspended duration.
6. Compare the pre/post kernel snapshots on the development machine and transcribe only the new matching lines. Do not clear the ring buffer or write a marker into it.

The target precision is seconds and bounded intervals, not milliseconds. Baseline A had no shared synchronization marker between the phone video and listener timestamps, so its visual and software timelines remain separate. They must not be aligned by assuming the button press coincided with `goingToScreenSaver`.

## Preconditions and observer-effect gate

- Keep the Phase 2A UI variants unchanged: passcode enabled, Wi-Fi on, portrait, not charging, no magnetic cover. Do not open a notebook or personal document.
- Use only the already-available shell transport. Prefer a pre-existing non-network console if one already exists and requires no setup or configuration change. Do not enable one for this experiment.
- If SSH is the only existing transport, leave the single foreground session idle except for listener output. Do not add keepalive traffic, pings, a second monitor, or another SSH session.
- Do not interpret SSH loss/recovery as suspend evidence. If SSH disconnects and the listener transcript is incomplete, the event-domain result failed even if post-wake kernel evidence is later available.
- Run CONTROL 0 first. Proceed to CONTROL 1 only after human review confirms the listener remained idle, the UI behaved normally, and no unexpected powerd event or option error appeared.

## CONTROL 0 — awake observer control

Purpose: determine whether the exact listener starts, remains quiet in a bounded awake interval, displays no option/permission error, and causes no visible behavior before it is trusted during a transition. This does not prove the listener has zero timing, networking, or power effect.

### Manual sequence

1. Navigate normally to the non-sensitive starting page, ensuring the device has just been used and is not close to its ordinary inactivity timeout.
2. Start phone video and show the Kindle and terminal.
3. Run the two exact commands below in order.
4. With no sleep request, cover action, pen/touch input, or additional shell command, observe for 60 seconds.
5. Press `Ctrl-C` once to exit only the foreground listener. Stop the video after the shell prompt returns.
6. Stop here if an event appears without an understood ordinary cause, an option is rejected, the listener exits on its own, the UI changes, or the prompt does not return normally.

### Exact commands

```sh
date -u '+PHASE2B-CONTROL0 %Y-%m-%dT%H:%M:%SZ'
lipc-wait-event -m -s 0 -t com.lab126.powerd goingToScreenSaver,outOfScreenSaver,exitingScreenSaver,readyToSuspend,wakeupFromSuspend
```

| Command | Safety | What it proves if successful |
| --- | --- | --- |
| `date -u ...` | READ_ONLY | Reads a UTC synchronization marker; it does not set the clock. |
| narrow `lipc-wait-event ...` | READ_ONLY | Shows the exact listener form can stay attached for the awake interval without an immediate error or observed UI transition. Silence does not prove event delivery. |

CONTROL 0 passes only if the observer remains running and quiet for 60 seconds, the Kindle remains normally awake, and one `Ctrl-C` returns to the prompt. It does not authorize CONTROL 1 automatically; the transcript and video receive a human review gate.

## CONTROL 1 — Baseline A, KOReader button cycle

Purpose: correlate one human button sleep/wake cycle across physical, powerd-event, and kernel evidence in the current observed environment.

### Manual sequence

1. After CONTROL 0 review, return to the non-sensitive KOReader page selected for Baseline A. Record battery/charging, Wi-Fi, orientation, passcode, cover, and KOReader-publisher context without changing them.
2. Start a new continuous phone video with the Kindle, button, and terminal visible as described above.
3. Run the PRE commands below. Preserve all output on the development machine; do not redirect it to the Kindle.
4. Run the foreground listener command. Confirm it remains running without an error.
5. After at least five seconds of stable starting-page video, press the physical power button once. Do not use a LIPC property, injected input, cover, or software suspend.
6. Record the first visible E-Ink activity, completion of replacement, and stable sleep image. Do not touch the device or send shell traffic.
7. Wait 180 seconds after the stable sleep image. This bounded window is intended to include the roughly minute-scale readiness interval seen in earlier non-target evidence without assuming that interval applies to this firmware. The actual kernel entry is determined only afterward from kernel evidence.
8. Press the physical power button once to wake. Record first visible wake activity and normal protected UI restoration. If appropriate, unlock with the passcode obscured and record restoration of the starting page.
9. After the UI is stable and any expected exit events have appeared, press `Ctrl-C` once to exit only the foreground listener.
10. Run the POST commands below and end the video after their output appears.
11. Copy the terminal transcript and video to the development-machine evidence workspace. Do not create an output file on the Kindle. Redact before sharing.

### Exact PRE commands

```sh
date -u '+PHASE2B-CONTROL1-PRE %Y-%m-%dT%H:%M:%SZ'
cat /sys/kernel/debug/suspend_stats
dmesg 2>&1 | grep -E 'PM: suspend entry|PM: Preparing system for sleep \(mem\)|PM: Suspending system \(mem\)|Suspended for|wake up by|PM: Finishing wakeup|PM: suspend exit'
```

### Exact event command

```sh
lipc-wait-event -m -s 0 -t com.lab126.powerd goingToScreenSaver,outOfScreenSaver,exitingScreenSaver,readyToSuspend,wakeupFromSuspend
```

### Exact POST commands

```sh
date -u '+PHASE2B-CONTROL1-POST %Y-%m-%dT%H:%M:%SZ'
cat /sys/kernel/debug/suspend_stats
dmesg 2>&1 | grep -E 'PM: suspend entry|PM: Preparing system for sleep \(mem\)|PM: Suspending system \(mem\)|Suspended for|wake up by|PM: Finishing wakeup|PM: suspend exit'
```

### Command review

| Command | Safety | Read-only basis | Evidentiary role and limits |
| --- | --- | --- | --- |
| `date -u ...` | READ_ONLY | `date` has no clock-setting argument. | Anchors video/transcript to device UTC; clock accuracy remains a limitation. |
| `cat /sys/kernel/debug/suspend_stats` | READ_ONLY | Phase 2A found the fixed path readable and mode `r--r--r--`; `cat` opens it only for reading. | A pre/post field change may corroborate a kernel-recorded success/failure. Preserve raw field names; it does not measure depth/current. |
| `dmesg 2>&1` | READ_ONLY | Reads the existing kernel ring buffer; it omits `-c` and any console-level/configuration option. | Supplies kernel entry/resume evidence. Old matching lines require pre/post comparison; finite ring-buffer retention is a limitation. |
| `grep -E ...` | READ_ONLY | Filters the read stream and writes no device file. | Bounds output to PM transaction markers already observed on target. A non-match does not prove the event did not occur. |
| narrow `lipc-wait-event ...` | READ_ONLY | Subscription-only evidence and argument review are documented above. | Orders selected powerd broadcasts and payloads; names do not prove suspend or rendering. |

## Baseline A — KOReader / current observed environment

Classification: FACT — FACT-015, OBS-002

The accepted trial ran with KOReader active on the UI-identified Kindle Scribe running firmware 5.19.5. It is successful software/kernel evidence for that environment and is not a pure-stock baseline.

### Software and kernel timeline

The listener timestamp prefixes behaved like local wall-clock values. The adjacent device and kernel UTC timestamps show an observed UTC−07:00 relationship for this trial. This does not define `-t` behavior on other binaries or firmware.

| Marker | Observed local wall-clock-like time | Relation within the software/kernel timeline |
| --- | --- | --- |
| `goingToScreenSaver 2` | 20:06:34.131585 | event sequence start |
| first `readyToSuspend 10` | 20:07:34.140377 | 60.009 seconds after `goingToScreenSaver` |
| final `readyToSuspend 1` | 20:08:19.196563 | 105.065 seconds after `goingToScreenSaver` |
| kernel `mem` suspend entry | 20:08:29.360989 | 10.164 seconds after final observed readiness event |
| kernel suspend exit | 20:09:52.350269 | after kernel-reported suspended interval |
| `wakeupFromSuspend 83` | 20:09:52.369737 | 0.019 seconds after kernel exit |
| `outOfScreenSaver 1` | 20:09:52.550583 | 0.200 seconds after kernel exit |
| `exitingScreenSaver` | 20:09:52.968467 | 0.618 seconds after kernel exit |

The suspend-success counter changed from 76 to 77. The failure counter and all recorded suspend/resume failure fields remained zero. The kernel reported `Suspended for 82.711 seconds` and recorded a complete `mem` suspend/resume transaction.

**FACT:** The powerd/kernel ordering in the table occurred on this Scribe 5.19.5 trial with KOReader active.

**INFERENCE:** The repeated `readyToSuspend` payloads may represent a countdown or repeated readiness/defer process.

**UNKNOWN:** The precise semantics of the readiness payloads and their repeated schedule.

**INFERENCE:** The `wakeupFromSuspend` payload 83 is consistent with the kernel-reported 82.711-second suspended interval.

**UNKNOWN:** Whether the event payload is formally defined as suspended duration on this firmware or across Kindle versions.

### Visual timeline

| Marker | Video time | Delta from physical button |
| --- | --- | --- |
| physical power-button press | 1.67 seconds | — |
| illumination darkens | 2.05 seconds | +0.38 seconds |
| black/white flash and KOReader sleeping box appear | 2.10 seconds | +0.43 seconds |

**FACT:** These physical observations occurred during the accepted KOReader trial.

**UNKNOWN:** No shared synchronization marker relates the phone-video clock to the listener clock, so the visual markers cannot be placed before or after `goingToScreenSaver` from this evidence.

**UNKNOWN:** Physical wake-button, first visible wake activity, and stable restored-UI times were not available from the retained evidence. No repeat is required solely to fill them.

### Acceptance boundary

Baseline A establishes the selected powerd-to-kernel ordering, a successful kernel `mem` transaction, and the independent KOReader visual replacement timing for this one target trial. It does not identify the screen writer or panel-refresh owner, establish the stock Amazon replacement path, measure electrical power, or synchronize the visible replacement with a powerd event.

## Success criteria and interpretation

Baseline A is accepted as successful software/kernel evidence because it captured the selected powerd events, a new complete kernel `mem` suspend/resume sequence, pre/post suspend statistics, UTC context, and an independent physical sleep-transition video. Missing physical wake and restored-UI video times limit the visual domain but do not invalidate the accepted software/kernel evidence; no repeat is required solely for those fields.

The evidence worksheet preserves unavailable markers as UNKNOWN rather than filling gaps by assumption:

| Marker | Source | Observed time / interval | Confidence |
| --- | --- | --- | --- |
| physical sleep press | video | 1.67 seconds | high |
| illumination darkens | video | 2.05 seconds (+0.38) | high |
| flash / KOReader sleeping box | video | 2.10 seconds (+0.43) | high |
| each powerd event and payload | listener | recorded above | high for occurrence/order; payload semantics UNKNOWN |
| kernel `mem` suspend entry | dmesg | 20:08:29.360989 local equivalent | high |
| kernel resume / suspend exit | dmesg | 20:09:52.350269 local equivalent | high |
| physical wake press | video | UNKNOWN | unavailable |
| first visible wake activity | video | UNKNOWN | unavailable |
| stable protected UI / starting page | video | UNKNOWN | unavailable |

### Results that falsify current timing assumptions

The current working model places `goingToScreenSaver` before a pre-suspend display-replacement interval and `readyToSuspend` before kernel suspend. It is falsified or materially narrowed if this target shows any of the following with adequate correlation:

- visible replacement starts before `goingToScreenSaver` is published;
- visible replacement starts or completes only after `readyToSuspend`;
- the physical replacement completes only at or after kernel `mem` suspend entry;
- kernel `mem` suspend entry precedes the observed `readyToSuspend` event;
- events repeat, are absent, or occur in an order incompatible with one linear transition;
- visible restoration precedes kernel resume or occurs materially later than the screensaver-exit events.

Failure to reach kernel suspend while SSH/listener is present is not proof that normal firmware lacks suspend; it indicates an observer/timing confound or an incomplete wait and stops this design from answering the ordering question.

## KOReader confounder and later control condition

This first trial is reported as **Baseline A — KOReader / current observed environment**. It establishes what happened on the target while KOReader's `reader.lua` and publisher were present. It cannot attribute a replacement, event consequence, timing delay, or wake redraw exclusively to Amazon components, and it cannot be called a stock control.

Do not stop, kill, disable, signal, or modify KOReader for Phase 2B. Before a later cleaner condition is even designed, collect evidence that:

1. the publisher's presence/absence can be checked with the already-reviewed targeted D-Bus owner query;
2. a normal user-facing KOReader exit path, not a service/process control, removes the publisher and process;
3. that exit path does not change powerd, blanket, pillow, winmgr, passcode, display, Wi-Fi, or startup configuration;
4. no KOReader helper remains registered after that ordinary exit;
5. the same UI, battery/charging, Wi-Fi, orientation, cover, passcode, wait interval, and observer conditions can be repeated.

Those are evidence requirements, not approved device steps. A later control would need its own design and safety review.

## Stop conditions

Stop the current control and do not substitute another tool if:

- the observer requires any property write or event send;
- tracing, tracepoints, logging, or kernel configuration must be enabled or changed;
- a service or process other than the newly started foreground observer would need restart, stop, start, or a signal;
- framebuffer access or a screen-refresh command is required;
- an option is rejected or the command's behavior differs from the reviewed form;
- the observer triggers an unexpected event, visible change, repeated wake, or other behavior;
- SSH disconnects or output is incomplete;
- the device does not wake normally after the single ordinary wake press;
- the normal protected UI does not return;
- privacy-safe video/transcript capture cannot be maintained.

If the device fails to wake normally, do not issue experimental shell commands or repeated injected actions. Stop the experiment and use only the owner's pre-existing recovery procedure outside this specification.

## What Phase 2B can and cannot support

### CAN support

- the observed software/kernel order and approximate timing windows among the five selected powerd events and kernel `mem` suspend/resume in Baseline A;
- the separate physical timing from the sleep-button press to the KOReader sleeping box, without aligning it to the listener clock;
- whether each selected event is observed on this target during that one cycle, including repeats and payloads;
- whether the cycle reaches a kernel-recorded `mem` suspend/resume transaction while the observer is present;
- whether the long-lived listener/transport survives the cycle well enough to yield a continuous event record;
- evidence that a current pre-suspend timing assumption is false.

### CANNOT support

- which process writes framebuffer pixels, renders the image, or submits the E-Ink refresh;
- whether framebuffer memory or unchanged physical panel contents survive suspend/resume;
- electrical low-power consumption or the lowest hardware residency depth;
- that `readyToSuspend`, `wakeupFromSuspend`, screensaver state, SSH loss, or a log label alone proves suspend;
- pure-stock timing or causality while the KOReader publisher/helper is present;
- behavior for cover, RTC, stylus, touch, USB, network, long-sleep, no-passcode, ads, another orientation, another Scribe generation, or another firmware;
- safety or correctness of any future StayInk intervention.

## Decision gate

Phase 2B Baseline A is complete and accepted as successful software/kernel evidence. CONTROL 0 and CONTROL 1 used only the manually audited READ_ONLY command set and normal physical button actions; no tracing, framebuffer access, executable probe, or StayInk intervention was used. The result remains explicitly scoped to KOReader on this Scribe 5.19.5 environment and to a potentially suspend-perturbing SSH observer. Stock replacement timing, display ownership, electrical low-power depth, synchronized visual-to-event timing, and wake visuals remain outside what this trial proves.
