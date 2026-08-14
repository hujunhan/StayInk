# Facts

These are deliberately narrow observations. None of them, alone or together, proves where stock Scribe firmware draws its sleep screen or that a proposed intervention preserves low-power suspend.

## FACT-001 — KOReader subscribes to distinct powerd events

- Classification: FACT
- Source: SRC-006
- Repository / commit: `koreader/koreader-base` @ `809c16e24427fe4a722a56ea1e4f9b32372aac95`
- Evidence location: `input/input-kindle.h` :: `generateFakeEvent`
- Kindle model / firmware: generic Kindle integration / UNKNOWN
- Confidence: high
- Relevance: exposes observable boundaries around screensaver entry, suspend readiness, wake, and screensaver exit.
- Evidence: the Kindle input monitor starts `lipc-wait-event` for `com.lab126.powerd` and separately maps `goingToScreenSaver`, `outOfScreenSaver`, `exitingScreenSaver`, `readyToSuspend`, and `wakeupFromSuspend` into synthetic input events.
- Verification: inspected the pinned source at lines 118–199; no code was executed.

## FACT-002 — KOReader handles screensaver entry separately from suspend readiness and wake

- Classification: FACT
- Source: SRC-004
- Repository / commit: `koreader/koreader` @ `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`
- Evidence location: `frontend/device/kindle/device.lua` :: `intoScreenSaver`, `outofScreenSaver`, `wakeupFromSuspend`, `readyToSuspend`
- Kindle model / firmware: generic Kindle integration / UNKNOWN
- Confidence: high
- Relevance: shows that one Kindle application does not treat “show screensaver” and “suspend” as the same callback.
- Evidence: separate handlers show or close KOReader's screensaver, record readiness timing, account for time after wake, and redraw on screensaver exit.
- Verification: inspected the pinned source at lines 756–853; this is a fact about KOReader's state handling, not stock implementation.

## FACT-003 — A Scribe log contains an ordered screen/suspend/wake event sequence

- Classification: FACT
- Source: SRC-008
- Repository / commit: issue evidence / NOT APPLICABLE
- Evidence location: issue 15757 attachment `crash.log`; event lines timestamped 2026-07-29
- Kindle model / firmware: Kindle Scribe 2022 / 5.19.4, as identified by the reporter
- Confidence: medium
- Relevance: confirms that a recent Scribe running KOReader exposes more than one observable stage around a long sleep.
- Evidence: one repeated sequence records `intoScreenSaver` at 09:08:04, `readyToSuspend` callbacks beginning at 09:09:04, `wakeupFromSuspend 3488` at 10:08:08, then `outofScreenSaver` at 10:08:08. Other sequences show a wake event while powerd remains in `screenSaver`, followed by another readiness cycle.
- Verification: inspected the attached user log as text; event names match FACT-001. The log does not independently measure current draw or kernel power state.

## FACT-004 — KOReader requests a power-button transition through powerd

- Classification: FACT
- Source: SRC-004
- Repository / commit: `koreader/koreader` @ `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`
- Evidence location: `frontend/device/kindle/powerd.lua` :: `KindlePowerD:toggleSuspend`
- Kindle model / firmware: Kindle integration with LIPC / UNKNOWN
- Confidence: high
- Relevance: identifies a power-management request path while leaving the meaning of downstream stock states open.
- Evidence: with a LIPC handle, KOReader writes integer `1` to `com.lab126.powerd` property `powerButton`; its fallback invokes `powerd_test -p`.
- Verification: inspected lines 242–247. The function name does not prove that every invocation reaches genuine low-power suspend.

## FACT-005 — FBInk separates framebuffer writes from panel refresh requests

- Classification: FACT
- Source: SRC-005
- Repository / commit: `KindleModding/FBInk` @ `13872c056d050f7079770a126ec37acd952afaba`
- Evidence location: `fbink.c` :: `memmap_fb`, `refresh`, `refresh_compat`, `fbink_refresh`; `fbink.h` :: `FBInkConfig.no_refresh`
- Kindle model / firmware: supported Linux framebuffer targets / UNKNOWN
- Confidence: high
- Relevance: a future experiment must distinguish changing memory from causing a visible E-Ink update.
- Evidence: FBInk maps framebuffer memory read/write, permits drawing with refresh suppressed, and exposes a refresh-only public API.
- Verification: inspected lines 3815–3915, 6000–6026, and 9858 onward plus the public header. This does not show what stock Scribe software does.

## FACT-006 — FBInk and KOReader select MediaTek-specific display handling for Scribe

- Classification: FACT
- Sources: SRC-004, SRC-005
- Repository / commits: `koreader/koreader` @ `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`; `KindleModding/FBInk` @ `13872c056d050f7079770a126ec37acd952afaba`
- Evidence location: KOReader `frontend/device/kindle/device.lua` :: `KindleScribe`, `KindleScribe:init`; FBInk `fbink_device_id.c` Scribe device cases and `fbink.c` :: `refresh_kindle_mtk`
- Kindle model / firmware: Scribe device identifiers / UNKNOWN
- Confidence: high that the projects use this path; medium for the projects' hardware labels
- Relevance: old non-MediaTek refresh behavior cannot be assumed to apply to Scribe.
- Evidence: both projects flag Scribe as MTK. FBInk maps first-generation Scribe identifiers to Barolo/Bellatrix3 and sends MTK refreshes with `MXCFB_SEND_UPDATE_MTK`; KOReader constructs its Scribe screen with `framebuffer_mxcfb` and enables MTK fast mode.
- Verification: inspected the pinned model tables and display initialization. These names are community-project mappings, not Amazon documentation.

## FACT-007 — KPM defaults ordinary package payload storage under `/mnt/us`

- Classification: FACT
- Source: SRC-001
- Repository / commit: `KindleModding/KPM` @ `ffa767fffadd731bd59f2bca8c83231f4fc0ab2d`
- Evidence location: `paths.md`; `meson.options` :: `db_path`, `pkg_path`; `src/install.c` :: `Internal_InstallItem`; `package/install.sh`
- Kindle model / firmware: KPM-supported environments / UNKNOWN
- Confidence: high
- Relevance: provides a reference-code location that does not require putting a future StayInk payload on the root filesystem.
- Evidence: KPM defaults its database to `/mnt/us/kmc/kpm/kpm.db`, packages to `/mnt/us/kmc/kpm/packages`, extracts there, and runs a package's `install.sh` from its extracted directory. KPM's own platform install script separately copies KPM files to `/var/local/kmc`.
- Verification: inspected the path documentation, build defaults, install flow at lines 603–659, and KPM platform install script at lines 42–50.

## FACT-008 — The reviewed KPM interface defines install, launch, and uninstall hooks

- Classification: FACT
- Sources: SRC-001, SRC-002, SRC-003
- Repository / commits: KPM @ `ffa767fffadd731bd59f2bca8c83231f4fc0ab2d`; example @ `fa4d169716d3f3f07cd2ac215002014a9b600d4b`; docs @ `cf4457c4808636dac5e06d2f2761cb4632efe66d`
- Evidence location: KPM `README.md`, `src/install.c`, `src/uninstall.c`, `cli/main.c`; example hook files; docs `content/kindle-dev/kpm/creating-a-package.md`
- Kindle model / firmware: KPM-supported environments / UNKNOWN
- Confidence: high for the reviewed KPM interface
- Relevance: packaging lifecycle must not be confused with the Kindle power lifecycle.
- Evidence: the reviewed interface invokes `install.sh` after extraction, `launch.sh` on explicit launch, and `uninstall.sh` before deletion. No power-event or background-service hook was found in those interfaces.
- Verification: inspected implementation, documentation, and example. The negative finding is limited to those reviewed interfaces; it does not prove that no separate background mechanism exists elsewhere.

## FACT-009 — KindleModding package guidance forbids rootfs writes by hooks

- Classification: FACT
- Source: SRC-003
- Repository / commit: `KindleModding/kindlemodding.github.io` @ `cf4457c4808636dac5e06d2f2761cb4632efe66d`
- Evidence location: `content/kindle-dev/kpm/creating-a-package.md` :: package scripts and scriptlets
- Kindle model / firmware: modern KPM guidance / UNKNOWN
- Confidence: high
- Relevance: aligns a future package with StayInk's root-filesystem safety rule.
- Evidence: the page says hooks must not write or remount rootfs, calls rootfs modification unsupported, and recommends a `/mnt/us/documents` scriptlet that launches the package through `/var/local/kmc/bin/kpm`.
- Verification: inspected lines 60–80. This is ecosystem guidance, not a security boundary enforced by the package format.

## FACT-010 — E Ink describes its display medium as bistable

- Classification: FACT
- Source: SRC-007
- Repository / commit: NOT APPLICABLE
- Evidence location: E Ink “Benefits” page :: “Bi-Stability”
- Kindle model / firmware: general E Ink technology / NOT APPLICABLE
- Confidence: high for the general property; no Scribe-specific claim
- Relevance: makes display persistence during system sleep physically plausible without keeping the CPU awake.
- Evidence: the panel vendor states that an E Ink image remains without continuous display power and power is needed when the image changes.
- Verification: reviewed the first-party page on 2026-08-14. Controller initialization or firmware redraws can still change the visible Scribe image.

## FACT-011 — Recent Scribe users report wake-only display corruption after long sleeps in KOReader

- Classification: FACT
- Source: SRC-008
- Repository / commit: issue evidence / NOT APPLICABLE
- Evidence location: issue 15757 body and comments
- Kindle model / firmware: Scribe 2022 at 5.19.4 and 5.18.3; one commenter self-described a “Kindle Scribe gen 1 (2024)” at 5.17.2
- Confidence: medium
- Relevance: wake-time display operations on Scribe are a concrete compatibility risk for any intervention.
- Evidence: reporters describe left-side corruption appearing as the wake dialog is cleared after a long sleep; a full refresh or another sleep/wake removes it. Short sleeps reportedly do not reproduce it.
- Verification: reviewed issue text, images, comments, and one attachment. This is not reproduced independently and is not stock-UI evidence.

## FACT-012 — KOReader has a runtime path that unloads and reloads the blanket screensaver module

- Classification: FACT
- Source: SRC-004
- Repository / commit: `koreader/koreader` @ `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`
- Evidence location: `frontend/device/kindle/device.lua` :: `frameworkStopped`, `KindleTouch:exit`
- Kindle model / firmware: Kindle “no framework” launch mode / UNKNOWN
- Confidence: high for the code path; low for Scribe applicability
- Relevance: identifies one display-side control independent of KOReader's powerd event subscription, but not yet a safe StayInk mechanism.
- Evidence: KOReader writes `unload=screensaver` to `com.lab126.blanket` in its framework-stopped path and writes `load=screensaver` on exit.
- Verification: inspected lines 367–383 and 2044–2054. This path is conditional, state-changing, and not demonstrated here on stock Scribe firmware.
