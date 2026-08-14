# Kindle Scribe Evidence

## What is confirmed

### Community implementations classify Scribe as MTK

Classification: FACT about SRC-004 and SRC-005 — FACT-006

FBInk maps first-generation Scribe identifiers to `Barolo` / `Bellatrix3` and second-generation identifiers to `Pisco` / `Bellatrix3`. KOReader selects an MTK-capable `KindleScribe` implementation for both of its Scribe 1 and Scribe 2 serial sets. These are community mappings, not Amazon-confirmed hardware documentation.

### Recent Scribe event and display evidence exists

Classification: FACT — FACT-003, FACT-011

| Model | Firmware | Environment | Narrow observation | Applicability |
| --- | --- | --- | --- | --- |
| Scribe 2022 | 5.19.4 | KOReader 2026.07 | attached log separates screensaver entry, readiness callbacks, wake, and `outOfScreenSaver`; reporter observes corruption after long sleeps | confirmed for that report, not stock UI |
| Scribe 2022 | 5.18.3 | KOReader 2026.07 | another user reports similar long-sleep wake corruption | user report only |
| Scribe gen 1 | 5.17.2 | KOReader 2026.07 | another user reports the issue | user report only |
| Scribe 2022 | 5.17.3 | modified stock screensaver resources | lock continued while the current screen stayed visible when resource selection failed | unsafe, rootfs-modified case; suspend unmeasured |

The final row from SRC-009 demonstrates only that lock and visible replacement diverged on one modified device. It is not a safe recipe and does not prove genuine suspend.

### Cover events are part of the observed path

Classification: FACT about KOReader and the attached log — SRC-004, SRC-008, SRC-010

KOReader labels distinct hall suspend/wake sources, and the 5.19.4 attachment contains hall and button sequences. A separate KOReader issue about cover wake on Scribe was resolved by enabling KOReader's own magnetic-cover setting, not by a platform fix. Cover behavior therefore needs its own test case; it must not be assumed equivalent to the button path.

## What is not confirmed

- No stock Scribe source identifies the sleep-image renderer or its refresh call.
- No examined evidence measures actual Scribe power draw or kernel suspend depth while the current image remains visible.
- No examined evidence proves panel contents naturally survive short suspend, long suspend, or resume initialization on Scribe.
- No examined evidence identifies a safe LIPC property that suppresses only the display replacement.
- No stylus-specific interaction found in the scoped references establishes the suspend state machine.
- Behavior on one Scribe generation or firmware remains unconfirmed on every other combination.

## Next experiment: `kindle-device-probe`

This is a design for a future observation session, not an implementation or authorization to run it.

### Purpose

Compare one real Scribe's identity, powerd event ordering, display metadata, processes, and existing runtime facilities against the assumptions above. It must not attempt to suppress a screen, write a framebuffer, change a LIPC property, stop a process, load a module, remount a filesystem, or install persistence.

### Preconditions

- Record exact Scribe generation and firmware before interpreting results.
- Use a non-sensitive stock UI page; close every notebook and personal document.
- Remove the magnetic cover for the button trial. Run a separate cover trial only afterward.
- Ensure ordinary wake/reboot is known to work before observation.
- Define where output goes. Strict mode prints to an externally captured console. If that is unavailable, creating one new timestamped log under `/mnt/us/stayink-probe/` requires separate approval; that is reversible logging, not literally zero-write probing.

### Read-only inventory

Collect only readable information already exposed by the device:

- firmware and kernel identity from `/etc/prettyversion.txt`, `uname`, `/proc/cpuinfo`, and `/proc/cmdline`;
- mounted filesystems from `/proc/mounts`, without mounting or remounting anything;
- Scribe/display metadata from readable `/sys/class/graphics/fb0/*`, framebuffer variable/fixed-info queries, and device-node metadata—never pixel contents;
- supported sleep-state names from readable `/sys/power/state` and `/sys/power/mem_sleep` when present;
- process names and arguments for powerd, blanket, pillow, window manager, and GUI components, without signals or restarts;
- existing init-job names/configuration and read-only status, without starting, stopping, or reloading jobs;
- input topology from `/proc/bus/input/devices` and device links, without opening or grabbing input devices;
- LIPC publisher/property/event inventories using read-only probe operations; retrieve only documented readable powerd state/status properties;
- already-available kernel/system logs relevant to suspend and resume, without enabling new logging facilities or clearing logs.

Serial numbers, account identifiers, document names, and unrelated logs must be redacted. The probe must not read `/dev/fb0` pixels because the current display may contain private notebook or document content.

### Controlled observation sequence

1. Start a timestamped, subscription-only listener for the powerd event names in FACT-001 and periodically read the already-readable powerd state.
2. Record the baseline state for at least one minute without input.
3. Have the user press the physical power button once.
4. Leave the device untouched long enough to observe any readiness sequence and a conservatively long sleep interval.
5. Have the user wake it with the physical button and wait for the normal UI to settle.
6. Repeat once with a known magnetic cover, recording that as a separate trial.
7. Stop observation normally and remove any approved output file after it has been copied and reviewed.

The listener is observation-only. It must not synthesize `powerButton`, touch a timeout, schedule RTC wake, or prevent/abort/defer suspend.

### Success criteria

The probe succeeds if it produces a redacted timeline that correlates:

- user action;
- all observed powerd events and payloads;
- readable powerd states;
- relevant existing kernel suspend/resume messages;
- whether and when the visible display changes;
- button versus cover behavior;
- exact model, firmware, and display-controller metadata.

It must also report which requested data was unavailable rather than escalating privileges or changing configuration.

### What it can falsify

- If the Scribe does not expose the event set in FACT-001, KOReader's assumed observation interface is not portable to that firmware.
- If replacement occurs before any observable event, an event-triggered intervention may be too late.
- If readiness/wake ordering differs from SRC-008, that log cannot define the target device's sequence.
- If display/controller metadata does not match the MTK mappings, FBInk/KOReader assumptions require revision.

### Limit of the probe

An application event and elapsed timestamp do not prove low-power residency. If existing kernel messages are insufficient, a later experiment needs non-invasive external power measurement. That later measurement should be designed separately; the probe must not substitute an inference for proof.
