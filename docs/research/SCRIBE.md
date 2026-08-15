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
| Scribe 2022 | 5.18.3 | KOReader, version unstated | another user reports similar long-sleep wake corruption | user report only |
| “Kindle Scribe gen 1 (2024),” as self-described | 5.17.2 | KOReader, version unstated | another user reports the issue | user report only; model wording is ambiguous |
| Scribe 2022 | 5.17.3 | modified stock screensaver resources | lock continued while the current screen stayed visible when resource selection failed | unsafe, rootfs-modified case; suspend unmeasured |

The final row from SRC-009 demonstrates only that lock and visible replacement diverged on one modified device. It is not a safe recipe and does not prove genuine suspend.

### Cover events are part of the observed path

Classification: FACT about KOReader and the attached log — SRC-004, SRC-008, SRC-010

KOReader labels distinct hall suspend/wake sources, and the 5.19.4 attachment contains hall and button sequences. A separate KOReader issue about cover wake on Scribe was resolved by enabling KOReader's own magnetic-cover setting, not by a platform fix. Cover behavior therefore needs its own test case; it must not be assumed equivalent to the button path.

## What is not confirmed

- No stock Scribe source identifies the sleep-image renderer or its refresh call.
- No examined evidence measures actual Scribe power draw or establishes the lowest hardware residency depth while the current image remains visible.
- No examined evidence proves panel contents naturally survive short suspend, long suspend, or resume initialization on Scribe.
- No examined evidence identifies a safe LIPC property that suppresses only the display replacement.
- No stylus-specific interaction found in the scoped references establishes the suspend state machine.
- Behavior on one Scribe generation or firmware remains unconfirmed on every other combination.

## Phase 2A target-device reconnaissance — COMPLETE

Phase 2A was completed through the manually audited, read-only observations in `DEVICE_PROBE.md`. No executable probe, framebuffer access, service control, LIPC write, suspend request, or persistent device change was used.

### Observed target context

Classification: FACT — OBS-001

| Field | Observation |
| --- | --- |
| UI model wording | Kindle Scribe |
| Generation / physical model number | UNKNOWN / UNKNOWN |
| Firmware | 5.19.5 |
| Kernel / architecture | `4.9.77-lab126` / ARMv7 |
| Shell privilege | BusyBox shell as root with broad effective capabilities |
| Display interface | `hwtcon_v2`; 1860×2480 mode; 1872×4960 virtual size; 8 bits per pixel; stride 1872 |
| UI variants | passcode enabled; no Special Offers; portrait; no magnetic cover |
| Jailbreak environment | Véra and KPM |
| Third-party state | KOReader's `com.github.koreader.kindlepowerd` publisher was active |

The active KOReader publisher means the observed environment must not be called a pure stock baseline.

### Process and suspend evidence

Classification: FACT — FACT-013, FACT-014

The six named candidate publishers were mapped to their current executables and command lines. Blanket's observed command line contains the literal argument `screensaver`; KPPMainApp owns the `ScreenSaverListener` publisher; KOReader's publisher belongs to its active `reader.lua` process.

Classification: INFERENCE

The literal blanket argument and the independently observed blanket publisher make blanket a stronger candidate participant in the screen-replacement path. This is supported by FACT-013 and is consistent with, but does not prove, the blanket-related behavior in FACT-012.

Classification: UNKNOWN

Neither blanket's command line nor KPPMainApp's publisher name establishes who writes framebuffer pixels, submits the E-Ink panel refresh, renders the replacement, or acts first relative to powerd events.

Existing kernel messages record complete `mem` suspend/resume transactions on this target. They provide a kernel-level oracle independent of screensaver and powerd labels, but do not measure electrical power or establish the lowest hardware residency depth.

### Closure boundary

Phase 2A established the target identity available from the UI/kernel, firmware, privileges, mounts, installed observation tools, power/display/input/log interfaces, candidate publisher owners, and available kernel suspend evidence. The initially incomplete process inventory was closed by the targeted PID-to-procfs observation in FACT-013.

Phase 2A did not trace a controlled power-button transition, timestamp the visible replacement, attribute a framebuffer write or panel refresh, compare framebuffer and panel contents across suspend, measure current draw, or test a wake path. Those remain separate Phase 2B/2C design questions. No additional Phase 2A device commands are authorized or needed.
