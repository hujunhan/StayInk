# Kindle Scribe Phase 2B Power-Transition Trace Specification

Status: Baseline A COMPLETE; Baseline B attempt 2 accepted as successful software/kernel evidence in the reported stock-Notebook foreground condition; synchronized visual evidence remains UNKNOWN
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

For Baseline A, only the exact commands classified **READ_ONLY** in its procedure were proposed. Here, READ_ONLY means the command has no property-set, file-write, log-clear, trace-enable, suspend, input, framebuffer, or service-control operation. It can still create observer effects: the shell and listener consume CPU and memory, create a transient D-Bus subscription, may generate ordinary session/audit records, and—when carried over SSH—keep a network connection present.

Baseline B necessarily adds temporary **STATE_CHANGING** operations confined to `/mnt/us/stayink-observation/` plus narrowly targeted signals to processes created by the observer itself. Its separate command review and stop gate appear below. No command classified **UNCERTAIN** is approved. If the target rejects an option, requests configuration, or behaves differently from the reviewed form, record the error and stop; do not improvise a replacement.

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

## Baseline B temporary-observer design — stock Notebook foreground

Status: Revision 1 failed before observation; revision 2 attempt 1 was inconclusive; supplied attempt 2 captured successful software/kernel evidence

Trial label if the human gate passes: **Baseline B — stock Notebook foreground after ordinary KOReader exit**

This is not a proven pure-stock baseline. The observer is launched from KOReader-provided SSH, third-party software remains installed, and the absence of KOReader's publisher during the observation window cannot be established by the observer proposed here. The intended condition is only that the user exits KOReader through its normal UI and places an existing, non-sensitive stock Kindle Notebook in the foreground before the physical sleep request. Do not create or edit notebook content for this trial.

### Revision 1 gate result

Classification: FACT — OBS-003

At `2026-08-15T03:52:09Z`, the revision 1 supervisor on this UI-identified Kindle Scribe running firmware 5.19.5 reported:

```text
mkfifo: /mnt/us/stayink-observation/baseline-b/events.pipe: Operation not permitted
```

`observer.pid` was written, the detached job exited, and neither `listener.pid` nor `timer.pid` was created. The required gate therefore failed before KOReader exit, before a powerd subscription was launched, and before any Notebook sleep/wake observation. The PRE suspend-success count of 78 is not a Baseline B result.

**FACT:** FIFO creation at that exact `/mnt/us` path was rejected on the observed target and environment.

**UNKNOWN:** Whether every path or FIFO implementation on this firmware would behave identically. No broader filesystem claim is made, and no additional location will be tried because observer output is restricted to `/mnt/us/stayink-observation/`.

The original validation loop also failed to guard empty PID files. Empty expansions caused unintended reads of `/proc/cmdline` and `/proc/stat`. Those reads were non-mutating, but revision 2 rejects missing, empty, or non-numeric PIDs before constructing a `/proc/<PID>` path.

### Narrow source validation and residual uncertainty

**FACT:** SRC-004 at commit `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`, `plugins/SSH.koplugin/main.lua` lines 45–56, launches Dropbear with a PID file. Lines 108–147 implement shutdown by sending `TERM`, and optionally `KILL`, to processes whose immediate parent is the recorded Dropbear PID and then to Dropbear itself. Lines 167–193 say active connections may remain without force-close and show a Dropbear-only last-resort kill. Lines 327–335 describe the force-close setting as terminating active SSH sessions. This is evidence about the pinned KOReader implementation, not proof of the exact target package's complete descendant-cleanup behavior.

**FACT:** The same source lines 64–71 and 150–158 show that starting and stopping KOReader SSH changes firewall rules. Baseline B does not start, stop, reconfigure, or otherwise control that service; it uses the already-running connection and then the user's ordinary KOReader UI exit.

**INFERENCE:** A process launched with a new session/process group, no controlling terminal, ignored `SIGHUP`, and all standard streams redirected should survive ordinary terminal hangup and the immediate-child signaling visible in the pinned plugin.

**UNKNOWN:** Whether the target's packaged plugin, launcher, or another supervisor performs broader process-tree or cgroup cleanup when KOReader exits. Therefore successful detachment is a pre-exit experimental gate, not an assumption. If the observer nevertheless dies on ordinary KOReader exit, Baseline B fails safely and no timing conclusion follows.

### Why `nohup`, `setsid`, and a local timer are combined

- `nohup` establishes ignored `SIGHUP` before executing the observer. It does not create a new session, process group, or detach a controlling terminal, so it is insufficient by itself.
- `setsid` requests a new session and process group with no controlling terminal. It does not by itself protect against an explicit `SIGHUP`, so it is insufficient by itself.
- stdin is `/dev/null`; stdout and stderr are explicitly redirected under the run directory. This prevents terminal reads and prevents creation of a default `nohup.out` elsewhere.
- The observer writes its own PID after `setsid`, because the launching shell's `$!` may identify an intermediate implementation process if `setsid` forks.
- An external `timeout` wrapper is not approved. Availability and option/signal-forwarding behavior have not been demonstrated on this target, and different BusyBox/coreutils versions do not justify assuming equivalent child handling. The preflight may report whether it exists, but the design does not execute it.
- Revision 1 attempted event-driven termination through a FIFO under `/mnt/us`; OBS-003 disproved that mechanism on this target. Revision 2 writes listener output directly to `events.log` and uses one `/bin/sleep 360` child as its sole automatic termination trigger. It uses no polling loop, FIFO, `tail`, or wake-capable alarm.

**UNKNOWN:** Which clock the target's `/bin/sleep` ultimately uses and whether elapsed suspend time advances it. The 360-second bound is therefore a runnable-time bound, not a guaranteed 360 seconds of external wall time across kernel suspend. While the kernel is suspended no userspace cleanup can execute. After a normal wake, the remaining timer interval completes and stops the listener.

### Exact proposed sequence

1. Start continuous phone video. Record firmware, battery/charging, Wi-Fi, orientation, passcode, cover state, the Baseline B label, and a non-sensitive existing Notebook chosen in advance. Do not open or edit that Notebook yet.
2. While the already-running KOReader SSH session is available, run the availability preflight. Stop if `nohup`, `setsid`, `awk`, `grep`, `sed`, `tr`, or `readlink` is absent, or if any of the three exact executable paths fails its check. `timeout` is informational only. Do not retry `mkfifo`.
3. Create the fixed output directory. It must be empty. If it is not empty, stop and review it; do not overwrite or broadly remove anything.
4. Create and inspect the one-shot observer exactly as below. It is an experiment artifact under `/mnt/us`, not an installed daemon or reusable startup component.
5. Capture the PRE UTC marker, suspend statistics, and filtered existing kernel PM messages in the SSH transcript.
6. Launch the observer with `nohup` and `setsid`, then run the targeted detachment check. A human must confirm the observer has its own session/process group, `tty_nr=0`, ignored `SIGHUP`, expected executable/command lines, and only the recorded PIDs were inspected. If any condition is unclear, run the targeted abort and stop before exiting KOReader.
7. Keep the video continuous. Exit KOReader through its ordinary user-facing UI, navigate normally to the preselected stock Notebook without editing it, and show the stable page for at least five seconds.
8. Press the physical power button once. Record illumination change, first E-Ink activity, replacement completion, and stable sleep image. Wait at least 180 seconds after the stable sleep image so the approximately minute-scale readiness sequence observed in Baseline A has room to reach kernel suspend; this interval is not itself suspend evidence.
9. Press the physical power button once to wake. Record first visible wake activity, the protected UI, and restoration of the stock Notebook after a privacy-safe unlock. Stop if wake or UI restoration is abnormal.
10. Keep the stock UI stable for at least five seconds after restoration. Revision 2 deliberately does not react to event-file contents. Its single timer stops the listener after 360 target-runnable seconds from observer launch.
11. After the controlled cycle and visual recording are complete, reopen KOReader normally to regain its existing SSH transport. This post-cycle action is outside the evidence window. Do not use network loss or reconnection as a suspend oracle. If the timer has not completed, leave the session idle until `done` appears; do not shorten it with an improvised signal.
12. Run the targeted completion check only after `done` exists, then take the POST kernel snapshots. Copy the entire run directory and phone video to the development machine. Preserve raw output and redact only the shared copy.
13. After the copy is verified and every recorded observer PID is gone or belongs to an unrelated reused PID, run the exact cleanup commands.

### Availability preflight

```sh
for c in nohup setsid awk grep sed tr readlink; do
    printf '%s: ' "$c"
    command -v "$c" || printf 'NOT FOUND\n'
done
printf 'timeout (informational, not used): '
command -v timeout || printf 'NOT FOUND\n'
for p in /bin/sh /bin/sleep /usr/bin/lipc-wait-event; do
    if [ -x "$p" ]; then
        printf '%s: executable\n' "$p"
    else
        printf '%s: NOT EXECUTABLE\n' "$p"
    fi
done
```

**SAFETY CLASSIFICATION:** READ_ONLY.

This resolves command availability only. It does not execute `timeout`, subscribe to events, alter the filesystem, enumerate processes, or prove detachment behavior.

### Command classification summary

| Exact operation | Classification | Basis |
| --- | --- | --- |
| availability preflight; PRE/POST `date`, `cat`, `dmesg`, and `grep`; fixed-PID `/proc` reads; artifact reads | READ_ONLY | Reads command/path availability or already-existing metadata and logs; no setting, clearing, broad process enumeration, or device write. |
| narrow `/usr/bin/lipc-wait-event ...` invocation by itself | READ_ONLY | Same already-approved subscription-only publisher/event form used in Baseline A. |
| `umask`, fixed-directory `mkdir`, heredoc creation, `chmod`, and output creation/redirection | STATE_CHANGING | Creates only temporary regular-file artifacts below `/mnt/us/stayink-observation/baseline-b/`; no rootfs or startup path. |
| `nohup setsid /bin/sh ... &` supervisor launch and `/bin/sleep 360` timer | STATE_CHANGING | Creates bounded transient processes and a D-Bus observer; it does not control a service or request a power transition. |
| identity-checked `kill -INT` of the listener and `kill -TERM` of the timer's own sleep child | STATE_CHANGING | Signals only PIDs created and recorded by this observer; no stock, KOReader, or Amazon service PID is targeted. |
| exact-file `rm -f` and empty-directory `rmdir` cleanup | STATE_CHANGING | Irreversibly deletes only enumerated temporary artifacts after copy and PID verification. |
| external `timeout` execution or any workaround not shown here | UNCERTAIN | Not approved because target implementation and signal-forwarding semantics were not established. |

No command classified UNCERTAIN is part of the executable sequence.

### Prepare the empty run directory

```sh
OBS=/mnt/us/stayink-observation
RUN="$OBS/baseline-b"
umask 077
mkdir -p "$RUN"
ls -la "$RUN"
```

**SAFETY CLASSIFICATION:** STATE_CHANGING.

`mkdir -p` may create only the two named directories under `/mnt/us`; `umask` affects only files subsequently created by this shell; `ls` is read-only. Proceed only if `baseline-b` is empty. No root-filesystem path, mount, service, or persistent startup location is touched.

### One-shot observer content

```sh
cat > "$RUN/observer.sh" <<'EOF'
#!/bin/sh

RUN=/mnt/us/stayink-observation/baseline-b
LISTENER_PID=
TIMER_PID=

umask 077
date -u '+%Y-%m-%dT%H:%M:%SZ' > "$RUN/start.utc"
printf '%s\n' "$$" > "$RUN/observer.pid"
: > "$RUN/events.log"

/usr/bin/lipc-wait-event -m -s 0 -t com.lab126.powerd goingToScreenSaver,outOfScreenSaver,exitingScreenSaver,readyToSuspend,wakeupFromSuspend \
    > "$RUN/events.log" 2> "$RUN/listener.stderr" &
LISTENER_PID=$!
printf '%s\n' "$LISTENER_PID" > "$RUN/listener.pid"

(
    SLEEP_PID=
    stop_sleep() {
        if [ -n "$SLEEP_PID" ]; then
            kill -TERM "$SLEEP_PID" 2>/dev/null
            wait "$SLEEP_PID" 2>/dev/null
        fi
        exit 0
    }
    trap stop_sleep TERM INT
    /bin/sleep 360 &
    SLEEP_PID=$!
    wait "$SLEEP_PID"
    : > "$RUN/timeout.fired"
    if [ -r "/proc/$LISTENER_PID/cmdline" ]; then
        LISTENER_CMD=$(tr '\000' ' ' < "/proc/$LISTENER_PID/cmdline")
        case "$LISTENER_CMD" in
            *lipc-wait-event*) kill -INT "$LISTENER_PID" 2>/dev/null ;;
        esac
    fi
) &
TIMER_PID=$!
printf '%s\n' "$TIMER_PID" > "$RUN/timer.pid"

wait "$LISTENER_PID"
LISTENER_STATUS=$?
kill -TERM "$TIMER_PID" 2>/dev/null
wait "$TIMER_PID" 2>/dev/null
if [ -f "$RUN/timeout.fired" ]; then
    STOP_REASON=timeout
else
    STOP_REASON=listener-exited-before-timeout
fi
date -u '+%Y-%m-%dT%H:%M:%SZ' > "$RUN/finish.utc"
printf '%s\n' "$LISTENER_STATUS" > "$RUN/listener.status"
printf '%s\n' "$STOP_REASON" > "$RUN/stop.reason"
printf '%s\n' complete > "$RUN/done"
exit 0
EOF
chmod 700 "$RUN/observer.sh"
sed -n '1,220p' "$RUN/observer.sh"
```

**SAFETY CLASSIFICATION:** STATE_CHANGING.

The heredoc creates one named file under the permitted directory, `chmod` limits its access, and `sed` displays it for manual audit. The content has no LIPC property write, service control, rootfs path write, framebuffer access, tracing operation, package action, startup integration, FIFO, or other special filesystem object. During execution it creates only the named regular-file run artifacts.

### PRE kernel snapshot

```sh
date -u '+PHASE2B-BASELINE-B-PRE %Y-%m-%dT%H:%M:%SZ'
cat /sys/kernel/debug/suspend_stats
dmesg 2>&1 | grep -E 'PM: suspend entry|PM: Preparing system for sleep \(mem\)|PM: Suspending system \(mem\)|Suspended for|wake up by|PM: Finishing wakeup|PM: suspend exit'
```

**SAFETY CLASSIFICATION:** READ_ONLY.

These are the already-approved synchronization and kernel-snapshot reads. They do not clear logs, set time, or change suspend state.

### Detached launch and mandatory human gate

```sh
nohup setsid /bin/sh "$RUN/observer.sh" </dev/null >"$RUN/supervisor.log" 2>&1 &
sleep 1
for f in observer.pid listener.pid timer.pid; do
    if [ ! -s "$RUN/$f" ]; then
        printf '\n%s: MISSING OR EMPTY; STOP\n' "$f"
        continue
    fi
    p=$(cat "$RUN/$f")
    case "$p" in
        ''|*[!0-9]*)
            printf '\n%s: INVALID PID %s; STOP\n' "$f" "$p"
            continue
            ;;
    esac
    printf '\n%s: %s\n' "$f" "$p"
    if [ ! -d "/proc/$p" ]; then
        printf 'PID is not running; STOP\n'
        continue
    fi
    readlink -f "/proc/$p/exe"
    printf 'cmdline: '
    tr '\000' ' ' < "/proc/$p/cmdline"
    printf '\n'
    grep -E '^(Name|PPid|SigIgn):' "/proc/$p/status"
    awk '{print "pid=" $1 " ppid=" $4 " pgrp=" $5 " session=" $6 " tty_nr=" $7}' "/proc/$p/stat"
done
```

**SAFETY CLASSIFICATION:** STATE_CHANGING for the launch; READ_ONLY for `sleep` and the three-PID `/proc` inspection.

The launch creates transient observer processes, a subscription, and output files. The inspection is deliberately limited to the three PIDs written by this run; it does not enumerate unrelated processes or inspect environment, memory, file descriptors, or open files.

The human gate passes only if all three files are present and non-empty, every PID is numeric and running, and:

- `observer.pid` identifies `/bin/busybox` or the expected `/bin/sh` implementation with `observer.sh` in its command line;
- `listener.pid` identifies `lipc-wait-event` with exactly the approved publisher and event list;
- the observer's `pid`, `pgrp`, and `session` values are equal and its `tty_nr` is `0`;
- `SigIgn` for the observer and listener includes the low-order `SIGHUP` bit; and
- no command failed and the observer has not already written `done`.

Do not infer a pass from `nohup` or `setsid` merely returning success. If the numeric `SigIgn` interpretation or any process identity is unclear, use the targeted abort below and stop.

### Process lifecycle

```text
KOReader reader.lua
  `- Dropbear server
       `- SSH session / interactive shell
            `- nohup + setsid launch
                 `- observer.sh  [new SID/PGID, tty_nr=0, SIGHUP ignored]
                      |- lipc-wait-event --narrow-five-events
                      |    `- stdout -> events.log
                      `- timer subshell
                           `- /bin/sleep 360

ordinary KOReader UI exit
  -> Dropbear/session may terminate
  -> detached observer is expected, but not proven, to remain

normal wake; event lines remain observational only
  -> sleep completes after 360 target-runnable seconds
  -> timer verifies listener cmdline, sends SIGINT
  -> observer records timeout completion and exits

listener exits unexpectedly before timer
  -> observer cancels/reaps timer
  -> stop.reason records listener-exited-before-timeout
  -> trial fails; no replacement observer is improvised
```

### Output and completion check

All deliberate observer output is confined to:

```text
/mnt/us/stayink-observation/baseline-b/
  observer.sh
  start.utc
  observer.pid
  listener.pid
  timer.pid
  events.log
  listener.stderr
  supervisor.log
  timeout.fired       # present only if the timer fired
  finish.utc
  listener.status
  stop.reason
  done
```

`events.log` receives listener stdout directly and preserves its timestamp prefixes. On this target `-t` previously produced local wall-clock-like timestamps related to UTC by the observed timezone offset, but that relationship must be re-established from `start.utc` and the video marker rather than assumed. Regular-file buffering may delay visibility while the process is running; only the file after the listener's reviewed `SIGINT` exit is evidence.

After the cycle, use only these exact targeted reads:

```sh
RUN=/mnt/us/stayink-observation/baseline-b
for f in start.utc events.log listener.stderr supervisor.log finish.utc listener.status stop.reason done; do
    printf '\n===== %s =====\n' "$f"
    if [ -r "$RUN/$f" ]; then cat "$RUN/$f"; else printf 'MISSING\n'; fi
done
for f in observer.pid listener.pid timer.pid; do
    if [ ! -s "$RUN/$f" ]; then
        printf '%s: MISSING OR EMPTY\n' "$f"
        continue
    fi
    p=$(cat "$RUN/$f")
    case "$p" in
        ''|*[!0-9]*)
            printf '%s: INVALID PID %s\n' "$f" "$p"
            continue
            ;;
    esac
    if [ -d "/proc/$p" ]; then
        printf '%s PID %s currently exists; cmdline: ' "$f" "$p"
        tr '\000' ' ' < "/proc/$p/cmdline"
        printf '\n'
    else
        printf '%s PID %s is gone\n' "$f" "$p"
    fi
done
date -u '+PHASE2B-BASELINE-B-POST %Y-%m-%dT%H:%M:%SZ'
cat /sys/kernel/debug/suspend_stats
dmesg 2>&1 | grep -E 'PM: suspend entry|PM: Preparing system for sleep \(mem\)|PM: Suspending system \(mem\)|Suspended for|wake up by|PM: Finishing wakeup|PM: suspend exit'
```

**SAFETY CLASSIFICATION:** READ_ONLY.

Successful automatic termination requires `done`, `finish.utc`, `listener.status`, and `stop.reason`, plus no current matching observer/listener/timer command line at the recorded PIDs. A recorded PID may have been reused; an unrelated current command line means the observer is gone and must not be signaled. Missing `done`, an expected command line still present after the timer interval and at least 30 awake seconds, or an unexpected `listener.stderr` means stop and use only the targeted abort procedure.

### Targeted abort and cleanup

Before KOReader exit, or after wake if the expected listener still exists, stop only the recorded listener after revalidating its command line:

```sh
RUN=/mnt/us/stayink-observation/baseline-b
LPID=$(cat "$RUN/listener.pid")
if [ -r "/proc/$LPID/cmdline" ]; then
    LCMD=$(tr '\000' ' ' < "/proc/$LPID/cmdline")
    case "$LCMD" in
        *lipc-wait-event*) kill -INT "$LPID" ;;
        *) printf 'PID identity mismatch; no signal sent\n' ;;
    esac
fi
sleep 2
```

**SAFETY CLASSIFICATION:** STATE_CHANGING.

`SIGINT` targets only the observer's recorded child after identity validation. The wrapper should then reap that listener, cancel its timer, write completion metadata, and exit. Do not use `killall`, `pkill`, `KILL`, a service control, or an unverified PID. If this does not work, preserve the artifacts and stop rather than improvising.

Only after the output is copied and PID verification shows no matching observer process remains:

```sh
RUN=/mnt/us/stayink-observation/baseline-b
OBS=/mnt/us/stayink-observation
rm -f "$RUN/observer.sh" "$RUN/start.utc" "$RUN/observer.pid" "$RUN/listener.pid" "$RUN/timer.pid" "$RUN/events.log" "$RUN/listener.stderr" "$RUN/supervisor.log" "$RUN/timeout.fired" "$RUN/finish.utc" "$RUN/listener.status" "$RUN/stop.reason" "$RUN/done"
rmdir "$RUN"
rmdir "$OBS" 2>/dev/null || :
```

**SAFETY CLASSIFICATION:** STATE_CHANGING.

This irreversibly removes only the exact temporary artifacts named above. `rmdir` removes only empty directories; it will preserve the parent if any unrelated file exists. There is no wildcard, recursive removal, rootfs path, remount, or persistent configuration change.

### Observer effects and confounders

- The observer adds one LIPC/D-Bus subscriber, one shell supervisor, the listener, and one sleeping timer. The listener and completion metadata produce a small number of `/mnt/us` writes.
- Revision 2 uses a regular output file because OBS-003 rejected the FIFO design. Exact buffering and flush behavior remain target-dependent; the evidence is assessed only after reviewed `SIGINT` termination. Missing or truncated lines make the event domain incomplete.
- The timer has no polling loop and intentionally uses no RTC/alarm wake mechanism. It should not wake the device, but it consumes a PID and timer resource and may slightly perturb scheduling before suspend and after resume.
- `/mnt/us` is a userstore/FUSE path on this target. Its availability and write latency across suspend are not independently characterized. The design performs no deliberate writes while the kernel is suspended; event and completion writes occur in runnable userspace.
- KOReader, its SSH plugin, Wi-Fi, and the interactive shell are active during setup. Normal KOReader exit removes the live transport and changes the foreground application. This trial can compare a stock Notebook foreground condition with Baseline A, but cannot establish an unmodified stock boot or attribute differences solely to one helper.
- The observer's origin under KOReader remains a provenance confound even if it survives process detachment. Whether `com.github.koreader.kindlepowerd` is absent throughout the evidence window remains UNKNOWN in this design.
- SSH disappearance/reappearance is not suspend evidence. Wi-Fi activity before KOReader exit and after the trial can affect timing and power. No claim about electrical consumption follows.
- `nohup` and `setsid` protect against ordinary hangup/session coupling, not every possible explicit signal, cgroup cleanup, OOM kill, crash, reboot, or storage error.
- The phone video, Notebook page, shell transcript, and logs may expose personal content, network details, paths, or timestamps. Use a non-sensitive existing Notebook page; do not record passcode entry or share unredacted artifacts.
- Ordinary OS, shell-history, filesystem, or audit logging may occur outside the run directory as a side effect of normal device operation. The proposed observer deliberately creates no output elsewhere and does not alter those facilities.

### Baseline B revision 2 trial result — inconclusive

Classification: FACT — FACT-016, OBS-004

The owner reported the stock Kindle Notebook foreground condition. The detached revision 2 observer ran from `2026-08-15T04:06:40Z` until `2026-08-15T04:12:40Z`, exited through its timer with listener status 0, produced empty stderr/supervisor logs, and left no observer, listener, or timer PID running.

The listener prefixes again behaved like local wall-clock values with the previously observed UTC−07:00 relationship. Preserve that as a trial-scoped observation, not a universal definition of `-t`.

| Episode / marker | Listener time | Relation |
| --- | --- | --- |
| episode 1 `goingToScreenSaver 2` | 21:09:22.862479 | first captured episode begins |
| episode 1 `outOfScreenSaver 1` | 21:09:59.118818 | +36.256 seconds |
| episode 1 `exitingScreenSaver` | 21:09:59.567320 | +36.705 seconds |
| episode 2 `goingToScreenSaver 2` | 21:10:15.093455 | 15.526 seconds after episode 1 exit |
| episode 2 first `readyToSuspend 10` | 21:11:15.207734 | +60.114 seconds from episode 2 start |
| episode 2 repeated `readyToSuspend 10` | 21:11:35.240913 | +80.147 seconds |
| episode 2 repeated `readyToSuspend 10` | 21:12:05.278706 | +110.185 seconds |
| episode 2 final captured `readyToSuspend 10` | 21:12:35.319204 | +140.226 seconds |
| observer timer completion | 21:12:40 local equivalent | approximately five seconds after the final captured event |

No `wakeupFromSuspend` was captured. The post-trial kernel log contained no `PM: suspend entry` after the observer's `04:06:40Z` start; its newest matching transaction entered at `04:02:42.669813449Z` and exited at `04:03:45.082370298Z`, before the observer existed. The post-trial suspend-success value was 79 with all reported failure fields zero. The previously supplied PRE value of 78 was recorded at `03:51:48Z`, before that intervening `04:02:42Z` transaction, so the 78-to-79 change cannot be attributed to Baseline B.

**FACT:** Two powerd screensaver-event episodes were captured during the observer window, and the second produced repeated `readyToSuspend 10` events until the timer ended.

**FACT:** No kernel `mem` suspend/resume transaction was recorded during the observer window. Therefore neither `readyToSuspend` nor visible screen replacement establishes genuine suspend in this trial.

**UNKNOWN:** What caused the first screensaver episode and whether it was related to ordinary KOReader exit, stock UI navigation, or a physical button action. The event log alone does not identify its cause.

**UNKNOWN:** Why the second episode remained at payload 10 and did not reach kernel suspend during the captured interval. Candidate explanations include an ordinary power-management defer condition, observer/environment effects, or insufficient observation time; this evidence does not distinguish them.

**UNKNOWN:** Which software episode corresponds to the phone video. No shared synchronization marker has been supplied that safely aligns the relative video clock with either `goingToScreenSaver` timestamp.

The independent visual record shows:

| Visual marker | Video time | Delta from physical button |
| --- | --- | --- |
| physical power-button press | 1.00 seconds | — |
| illumination darkens | 1.50 seconds | +0.50 seconds |
| full black | 3.80 seconds | +2.80 seconds |
| full white | 3.98 seconds | +2.98 seconds |
| full black | 4.20 seconds | +3.20 seconds |
| full white | 4.34 seconds | +3.34 seconds |
| screensaver visible | 5.20 seconds | +4.20 seconds |

Physical wake-button, first visible wake activity, and stable restored-Notebook video times remain UNKNOWN. A repeat is not requested solely to fill those visual fields.

This trial supports the visual replacement sequence and the separate captured powerd episodes in the reported stock-Notebook foreground condition. It does not establish their mutual ordering, a completed kernel suspend, the reason suspend was deferred, or stock rendering/refresh ownership.

### Baseline B revision 2 attempt 2 — successful software/kernel evidence

Classification: FACT — FACT-017, OBS-005

This second supplied execution used the same revision 2 observer after the prior decision gate had said not to repeat it unchanged. The evidence is preserved because it occurred, but its success does not retroactively approve bypassing that gate. The owner again supplied it as the stock Kindle Notebook foreground condition after ordinary KOReader exit; the environment remains Véra/KPM and is not a proven pure-stock boot.

The observer ran from `2026-08-15T05:40:20Z` until `2026-08-15T05:48:18Z`, exited through its timer with listener status 0, produced empty stderr/supervisor logs, and left all three recorded PIDs gone. The listener's local wall-clock-like prefixes again have the observed UTC−07:00 relationship to kernel UTC for this trial.

| Marker | Local wall-clock-like time | Relation |
| --- | --- | --- |
| `goingToScreenSaver 2` | 22:40:58.346793 | event sequence start |
| first `readyToSuspend 10` | 22:41:58.363176 | +60.016 seconds |
| `readyToSuspend 8` | 22:42:03.369155 | +65.022 seconds |
| first `readyToSuspend 7` | 22:42:08.375699 | +70.029 seconds |
| repeated `readyToSuspend 7` | 22:42:28.418574 | +90.072 seconds |
| `readyToSuspend 6` | 22:42:33.428208 | +95.081 seconds |
| `readyToSuspend 2` | 22:42:38.430778 | +100.084 seconds |
| final `readyToSuspend 1` | 22:42:43.436594 | +105.090 seconds |
| kernel `mem` suspend entry | 22:42:53.394434 | +115.048 seconds; 9.958 seconds after final readiness event |
| kernel suspend exit | 22:44:51.694686 | after the kernel-reported suspended interval |
| `wakeupFromSuspend 118` | 22:44:51.711417 | 0.017 seconds after kernel exit |
| `outOfScreenSaver 1` | 22:44:51.820859 | 0.126 seconds after kernel exit |
| `exitingScreenSaver` | 22:44:52.335824 | 0.641 seconds after kernel exit |

The kernel recorded a complete transaction: entry at `2026-08-15 05:42:53.394433801 UTC`, device suspension, `Suspended for 117.998 seconds`, wake, resume, and exit at `05:44:51.694685807 UTC`. The post-trial suspend-success value was 82 and all supplied failure fields were zero. No immediately preceding suspend-statistics snapshot was supplied for this attempt, so no counter delta is attributed to it; the in-window kernel timestamps independently identify the transaction.

**FACT:** The selected powerd events bracketed a genuine kernel `mem` suspend/resume transaction in this attempt's reported stock-Notebook foreground condition.

**INFERENCE:** The `wakeupFromSuspend` payload 118 is consistent with the kernel-reported 117.998-second suspended interval.

**UNKNOWN:** Whether that payload is formally defined as suspended duration on firmware 5.19.5 or other versions.

**INFERENCE:** The repeated readiness payload sequence is consistent with a readiness/defer countdown that eventually reached 1 before kernel entry.

**UNKNOWN:** The formal meaning, units, publisher-side conditions, and reason for individual readiness payload values.

**FACT:** Baseline A and this attempt both emitted the payload sequence `10, 8, 7, 7, 6, 2, 1`, placed the first readiness event about 60 seconds after `goingToScreenSaver`, the final payload about 105 seconds after it, and kernel entry about 115 seconds after it.

**INFERENCE:** In these two trials, changing the foreground from KOReader to the reported stock Notebook condition did not materially change the observed powerd readiness schedule leading to kernel entry.

**UNKNOWN:** Whether that timing is stable across further cycles, other device states, or a provably third-party-free environment; the two trials do not establish causality.

The observer's `start.utc` to `finish.utc` interval was 478 seconds, while its timer requested 360 seconds and the kernel reported 117.998 suspended seconds. Within the one-second file timestamp precision, `478 − 117.998 ≈ 360.002` seconds.

**FACT:** In this target trial, the observer's `/bin/sleep 360` timer did not advance materially during the kernel-reported suspend interval. It therefore bounded approximately 360 seconds of runnable wall time, not 360 seconds of external elapsed time across suspend.

No phone-video timeline or shared synchronization marker was supplied for attempt 2. The earlier visual sequence belongs to the prior inconclusive attempt and must not be combined with this transaction. Physical replacement timing and visible restoration relative to attempt 2's powerd/kernel events remain UNKNOWN.

### Baseline B decision gate

Revision 1 remains rejected. Revision 2 attempt 1 was inconclusive; attempt 2 establishes the selected powerd-to-kernel ordering and a genuine kernel `mem` transaction in the reported stock-Notebook foreground condition. It also establishes the timer's suspend-excluding behavior for this target trial.

Baseline B is not complete for its central visual-ordering question because attempt 2 lacks synchronized visual evidence. Do not repeat the device experiment solely to fill that gap without a separately reviewed synchronization design. The available evidence still cannot identify the renderer or panel-refresh owner, prove framebuffer retention, establish electrical low-power depth, or qualify the environment as pure stock.
