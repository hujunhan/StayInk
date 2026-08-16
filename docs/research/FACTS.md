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

## FACT-013 — Six candidate publishers were mapped to processes on the target Scribe

- Classification: FACT
- Source: OBS-001
- Repository / commit: NOT APPLICABLE; direct target-device observation
- Evidence location: Phase 2A D-Bus `GetConnectionUnixProcessID` results followed immediately by selected `/proc/<PID>` metadata reads
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: Véra jailbreak with KPM; KOReader publisher active; not a pure stock baseline
- Confidence: high for the point-in-time publisher-to-process mapping
- Relevance: narrows the processes that can be correlated with display replacement in Phase 2B and identifies an active third-party confounder.
- Evidence:

| D-Bus publisher | Executable | Observed process / command line |
| --- | --- | --- |
| `com.lab126.powerd` | `/usr/bin/powerd` | `powerd`; `powerd -f` |
| `com.lab126.blanket` | `/usr/sbin/blanket` | `blanket`; `blanket -t screensaver langpicker blankwindow` |
| `com.lab126.pillow` | `/app/KPPPillow-2.0/bin/pillowd` | `pillowd`; `/app/KPPPillow-2.0/bin/pillowd -c /app/KPPPillow-2.0/static/config` |
| `com.lab126.winmgr` | `/usr/bin/awesome` | `awesome`; `awesome` |
| `com.lab126.KPPMainApp.ScreenSaverListener` | `/app/bin/KPPMainAppV2` | `KPPMainApp`; argv began `/app/bin/KPPMainApp` |
| `com.github.koreader.kindlepowerd` | `/mnt/us/koreader/luajit` | `reader.lua`; `./luajit ./reader.lua` |

- Narrow interpretation: blanket's observed command line contains the literal argument `screensaver`. KPPMainApp owns the `ScreenSaverListener` publisher. KOReader's publisher was owned by its active `reader.lua` process.
- Verification: reviewed the owner-supplied command output. The query was limited to the six named services and read only `exe`, `comm`, `cmdline`, and selected `status` fields. Publisher ownership, executable names, and command-line tokens do not establish framebuffer-write, E-Ink refresh, rendering, or event-ordering ownership.

## FACT-014 — Existing target kernel logs record complete `mem` suspend/resume transactions

- Classification: FACT
- Source: OBS-001
- Repository / commit: NOT APPLICABLE; direct target-device observation
- Evidence location: Phase 2A filtered read of the existing kernel ring buffer
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5; Linux `4.9.77-lab126`
- Environment: Véra jailbreak with KPM and an active KOReader publisher; not a pure stock baseline
- Confidence: high that the captured lines record kernel system suspend/resume transactions; low for electrical low-power depth
- Relevance: establishes an available kernel-level suspend oracle that is independent of screensaver and powerd event terminology.
- Evidence: captured sequences include `PM: suspend entry`, `Preparing system for sleep (mem)`, `Suspending system (mem)`, completed device/noirq suspend phases, a platform-reported suspended duration, completed resume phases, and `PM: suspend exit`.
- Verification: read pre-existing messages only; no suspend was requested by the probe. These lines establish kernel `mem` suspend/resume handling, but do not measure current draw, identify the lowest hardware residency state, or show that the current display survived unchanged.

## FACT-015 — A controlled KOReader trial orders powerd events around a kernel `mem` transaction

- Classification: FACT
- Source: OBS-002
- Repository / commit: NOT APPLICABLE; direct target-device observation
- Evidence location: accepted Baseline A record in `docs/research/POWER_TRACE.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5; Linux `4.9.77-lab126`
- Environment: Véra/KPM with KOReader active and its `com.github.koreader.kindlepowerd` publisher present; not a pure-stock baseline
- Confidence: high for the observed event/kernel order and successful kernel transaction; low for undocumented event-payload semantics and electrical low-power depth
- Relevance: establishes a target-specific pre-suspend event window and independent kernel transition evidence without changing power, display, service, or tracing state.
- Evidence:

| Marker | Observed local wall-clock-like time |
| --- | --- |
| `goingToScreenSaver 2` | 20:06:34.131585 |
| first `readyToSuspend 10` | 20:07:34.140377 |
| final `readyToSuspend 1` | 20:08:19.196563 |
| kernel `mem` suspend entry | 20:08:29.360989 |
| kernel suspend exit | 20:09:52.350269 |
| `wakeupFromSuspend 83` | 20:09:52.369737 |
| `outOfScreenSaver 1` | 20:09:52.550583 |
| `exitingScreenSaver` | 20:09:52.968467 |

- Kernel corroboration: `/sys/kernel/debug/suspend_stats` success changed from 76 to 77, all observed failure fields remained zero, and the kernel reported `Suspended for 82.711 seconds`.
- Visual record: the physical button press occurred at video 1.67 seconds, illumination darkened at 2.05 seconds (+0.38), and a black/white flash with the KOReader sleeping box appeared at 2.10 seconds (+0.43).
- Timestamp scope: the listener prefixes behaved like local wall time and had an observed UTC−07:00 relationship to adjacent UTC markers. No shared synchronization marker connected the phone-video clock to the listener clock, so the visual sequence cannot be aligned with `goingToScreenSaver`.
- Interpretation limits: repeated `readyToSuspend` payloads may reflect a countdown or readiness/defer process, but their precise semantics are UNKNOWN. Payload 83 is consistent with the 82.711-second kernel-reported suspended interval, but its formal definition is UNKNOWN. Physical wake and restored-UI video times are UNKNOWN. The trial does not identify a renderer, framebuffer writer, panel-refresh owner, stock Amazon timing, electrical current, or lowest hardware residency depth.

## FACT-016 — The first stock-Notebook Baseline B attempt did not reach kernel suspend

- Classification: FACT
- Source: OBS-004
- Repository / commit: NOT APPLICABLE; direct target-device and phone-video observation
- Evidence location: Baseline B revision 2 trial result in `docs/research/POWER_TRACE.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5; Linux `4.9.77-lab126`
- Environment: Véra/KPM; observer launched through KOReader SSH and reported stock Kindle Notebook foreground after ordinary KOReader exit; not a proven pure-stock boot
- Confidence: high for captured events, observer lifecycle, visual sequence, and absence of a kernel transaction during the observer window; low for episode causes and `readyToSuspend` payload semantics
- Relevance: demonstrates that visible replacement and `readyToSuspend` reports cannot be treated as proof of genuine suspend, and identifies an observation-bound/design issue before any StayInk intervention.
- Evidence: the detached observer ran from `04:06:40Z` through `04:12:40Z`, captured one short `goingToScreenSaver`/exit episode followed by a second episode with repeated `readyToSuspend 10` reports, and terminated cleanly by timer. The kernel log contained no suspend entry after observer start. Its newest transaction began at `04:02:42Z`, before the observer. The visual record independently showed the physical button at 1.00 seconds, illumination darkening at 1.50, alternating full-panel black/white states from 3.80 through 4.34, and a screensaver at 5.20.
- Verification: listener status was 0, stderr/supervisor logs were empty, all observer PIDs were gone, and post-trial suspend failure fields were zero. The supplied 78-to-79 success-counter change spans the earlier `04:02:42Z` transaction and is not attributed to Baseline B. No shared marker aligns the video with either software episode; episode causes, defer reason, wake visuals, renderer ownership, panel-refresh ownership, and electrical low-power depth remain UNKNOWN.

## FACT-017 — A later stock-Notebook Baseline B attempt brackets a kernel `mem` transaction

- Classification: FACT
- Source: OBS-005
- Repository / commit: NOT APPLICABLE; direct target-device observation
- Evidence location: Baseline B revision 2 attempt 2 in `docs/research/POWER_TRACE.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5; Linux `4.9.77-lab126`
- Environment: Véra/KPM; observer launched through KOReader SSH and reported stock Kindle Notebook foreground after ordinary KOReader exit; not a proven pure-stock boot
- Confidence: high for event/kernel ordering, complete kernel transaction, observer cleanup, and timer behavior; low for undocumented payload semantics; no visual-to-software confidence because no attempt-2 video timeline was supplied
- Relevance: establishes the stock-Notebook-condition software/kernel power ordering needed before a later visual correlation or StayInk intervention, while preserving the distinction between screensaver events and genuine suspend.
- Evidence: `goingToScreenSaver 2` was followed by readiness payloads `10, 8, 7, 7, 6, 2, 1`; kernel suspend entry followed the final payload by 9.958 seconds. The kernel reported 117.998 seconds suspended, then exited 0.017 seconds before `wakeupFromSuspend 118`; `outOfScreenSaver` and `exitingScreenSaver` followed. Listener exit status was 0, observer logs had no errors, and all recorded observer PIDs were gone.
- Timer observation: observer start-to-finish was 478 seconds for `/bin/sleep 360`; subtracting the kernel-reported 117.998-second suspend yields approximately 360.002 seconds. In this trial the timer did not advance materially during suspend.
- Verification limits: post-trial suspend success was 82 with zero supplied failure fields, but no immediately preceding counter snapshot was supplied, so no counter delta is claimed. The supplied kernel timestamps independently place the complete transaction inside the observer window. Formal event-payload meanings, attempt-2 visual timing, renderer/panel ownership, framebuffer retention, electrical power, and pure-stock applicability remain UNKNOWN.

## FACT-018 — KPPMainApp reacts to a wake-side screensaver-stop transition on the target

- Classification: FACT
- Source: OBS-006
- Repository / commit: NOT APPLICABLE; direct target-device observation
- Evidence location: Phase 3A filtered read of the existing `/var/log/messages`, distilled in `docs/research/DISPLAY_PATH_ATTRIBUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: Véra/KPM; KOReader has been present; not a proven pure-stock boot or the accepted Baseline B attempt-2 interval
- Confidence: high for the logged wake-side ordering and named KPPMainApp handlers; low for undocumented field semantics
- Relevance: confirms that KPPMainApp contains target-active screensaver lifecycle consumers, while preserving the distinction between lifecycle observation and sleep-screen presentation ownership.
- Evidence: at local-wall-clock-like `23:10:11.522`, powerd logged `SUSPENDED -> SCREEN SAVER` and the literal field `g_is_screensaver_drawn = (1)`; at `23:10:11.546` it logged `SCREEN SAVER -> ACTIVE`; and at `23:10:11.582` it sent `outOfScreenSaver`. At `23:10:11.595`, KPPMainApp logged receipt of `onOutOfScreenSaverEvent`, notification of an observer on screensaver stop, and `NotebooksControllerImpl::OnScreenSaverStop`. Other KPPMainApp clients then logged that the screensaver was turning off or that `inScreenSaver` became 0.
- Verification limits: the log does not identify the `com.lab126.KPPMainApp.ScreenSaverListener` publisher as the delivery mechanism, and it does not show KPPMainApp handling screensaver entry, choosing content, creating a window, writing pixels, or submitting a panel update. The earlier `UIManager:updateView` messages have no displayed-content or refresh-owner attribution. The powerd variable name and value report powerd's internal terminology, not proof that powerd or another named process rendered or refreshed the panel.

## FACT-019 — The target powerd binary embeds screensaver-drawn state and reset diagnostics

- Classification: FACT
- Source: OBS-007
- Repository / commit: NOT APPLICABLE; direct target-device static observation
- Evidence location: filtered `strings /usr/bin/powerd` output, distilled in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: Véra/KPM; awake, read-only static inspection; not a proven pure-stock boot
- Confidence: high for literal strings in the observed executable; medium for their module-level relationship; no call-graph confidence
- Relevance: makes powerd the strongest owner/reset candidate and narrows the next inspection toward the set-to-true trigger and any cross-component completion interface.
- Evidence: `/usr/bin/powerd` contains the literal diagnostics `g_is_screensaver_drawn = (%d)` and `screensaver_drawn is getting set to false`, both labelled with a `powerd:low_temp_state` context. It also contains `system/daemon/powerd/BLANKET_NAME`, `system/daemon/powerd/BLANKET_LOAD`, `goingToScreenSaver`, `readyToSuspend`, and low-temperature frontlight-management diagnostics.
- Verification limits: printable-string presence does not prove that a path executed, that the reset diagnostic directly surrounds an assignment, or that nearby strings share a caller. No string showed the set-to-true path. The blanket configuration keys do not prove that blanket reports draw completion, and the event-name proximity does not prove the field gates `readyToSuspend` or kernel suspend.

## FACT-020 — KPPMainAppV2 contains a named screensaver handler and readiness strings

- Classification: FACT
- Source: OBS-008
- Repository / commit: NOT APPLICABLE; direct target-device static observation
- Evidence location: filtered strings from the three remaining candidate binaries, distilled in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: Véra/KPM; awake, read-only static inspection; not a proven pure-stock boot
- Confidence: high for the literal strings and local demangling; no call-graph or runtime confidence
- Relevance: strengthens KPPMainApp as a screensaver lifecycle participant and makes its handler a focused static-inspection boundary, without attributing display completion or the powerd field.
- Evidence: `/app/bin/KPPMainAppV2` contains the mangled C++ symbol `_ZN3kpp3app17ApplicationModule18screenSaverHandlerEv`, which locally demangles to `kpp::app::ApplicationModule::screenSaverHandler()`. It also contains `readyToSuspend`, `screenSaver`, and `screenSaverTimeout`. The approved pattern produced no matching line for `/usr/sbin/blanket` or `/usr/bin/awesome`.
- Verification limits: the handler name does not define whether it handles entry, exit, image selection, presentation, or acknowledgement. String co-presence does not connect it to `readyToSuspend` or powerd. KPP's matched snippet-thumbnail and TOC draw-completion diagnostics explicitly concern those content operations; the generic clear-redraw diagnostic has no screensaver attribution. No selected match in blanket/Awesome does not prove that those processes lack the behavior.

## FACT-021 — Powerd writes its screensaver-drawn global after sending `goingToScreenSaver`

- Classification: FACT
- Source: OBS-010
- Repository / commit: NOT APPLICABLE; local static analysis of an owner-supplied target binary
- Evidence location: target `powerd` ARM Thumb code around virtual addresses `0x29726`–`0x29778`, `0x29d48`–`0x29d6c`, and `0x29e78`–`0x29e9a`, distilled in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: local read-only analysis of the copied, stripped `/usr/bin/powerd`; SHA-256 recorded in OBS-010
- Confidence: high for the static instruction, global-address, string-reference, and imported-call relationships; no runtime-execution or formal-semantic confidence
- Relevance: resolves set-to-true ownership and shows that the field is not an acknowledgement supplied by blanket, winmgr/Awesome, or KPPMainApp.
- Evidence: one powerd path formats and emits the `goingToScreenSaver` LIPC event with an integer payload through `LipcNewEvent`, `LipcAddIntParam`, and `LipcSendEvent`. After that send helper returns, powerd stores integer `1` at global address `0x65814` and logs `g_is_screensaver_drawn = (1)`. A separate powerd path logs `screensaver_drawn is getting set to false` and stores `0` to the same address; another path also stores `0` there and logs the generic value diagnostic. A later state-machine branch reads the same global and selects between two calls into an internal path containing the diagnostic `Device is in low power state. Changing flIntensity`.
- Verification limits: static order does not define whether `LipcSendEvent` waits for any subscriber work, why powerd names the flag “drawn,” or which transitions reach each store. The set-to-one path subsequently may call `device_can_suspend`, but the field is not passed to that call and instruction ordering alone does not establish a suspend gate. No setter site contains a framebuffer write, window/layer operation, E-Ink update request, or external presentation-completion callback.

## FACT-022 — KPP's named application-module screensaver handler is an event-metrics consumer

- Classification: FACT
- Source: OBS-010
- Repository / commit: NOT APPLICABLE; local static analysis of an owner-supplied target binary
- Evidence location: target `KPPMainAppV2` dynamic symbols and ARM Thumb code for `kpp::app::ApplicationModule::registerScreenSaverListener()` at `0x0110a8f4` and `screenSaverHandler()` at `0x01107308`, distilled in `docs/research/SCREENSAVER_DRAWN_ATTRIBUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: local read-only analysis of the copied, stripped `/app/bin/KPPMainAppV2`; SHA-256 recorded in OBS-010
- Confidence: high for the symbol, event-to-method-pointer, and direct-call relationships; no runtime-ordering confidence
- Relevance: removes this specifically named handler from the leading presentation-completion candidates while leaving other KPP modules and the stock presenter unresolved.
- Evidence: `registerScreenSaverListener()` registers both powerd `goingToScreenSaver` and `outOfScreenSaver` with the same `ApplicationModule::screenSaverHandler()` method pointer; it separately maps `readyToSuspend` to `ApplicationModule::deviceSuspendHandler(int)`. `screenSaverHandler()` toggles an object-local byte, records/accumulates elapsed system-clock time, and calls `ApplicationModule::emitDeviceEngagementMetrics(..., bool)`.
- Verification limits: this finding applies to the named `ApplicationModule` handler, not all KPPMainApp screensaver-related classes. It does not exclude another KPP module from choosing content or notifying observers, and it does not identify blanket, Awesome, Pillow, the framebuffer writer, or the panel-refresh owner.

## FACT-023 — The target blanket executable is a loader-driven X11 event-loop launcher

- Classification: FACT
- Source: OBS-011
- Repository / commit: NOT APPLICABLE; local static analysis of an owner-supplied target binary
- Evidence location: target `blanket` dynamic section, dynamic imports, embedded option help, and ARM Thumb main-loop code at virtual addresses `0x1c6c`–`0x2160`, distilled in `docs/research/DISPLAY_PATH_ATTRIBUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: local read-only analysis of the copied, stripped `/usr/sbin/blanket`; SHA-256 recorded in OBS-011
- Confidence: high for the ELF dependency, option-help, imported-call, and main-loop relationships; no runtime-execution or shared-library implementation confidence
- Relevance: strengthens blanket as a module host and X11 presentation-path coordinator while moving pixel production and update submission out of the demonstrated launcher code and into an unresolved shared-library/loader boundary.
- Evidence: the executable directly depends on `libblanket.so.1.0`, X11, Lab126 graphics/window utilities, Cairo, rendering, LIPC, and related libraries. Its embedded help defines `-t loader` as the “Name of loader(s) to initialize with blanket,” allowing a quoted, space-separated value when multiple loaders are supplied. Its option path passes the `-t` argument to `blanket_event_parse_loader`. After `blanket_init`, the main loop polls a named blanket wakeup pipe and the X11 connection, converts pending X events with `blanket_event_enqueue_x11`, and invokes `blanket_event_dispatch`; these `blanket_*` operations are undefined dynamic imports. `libblanket.so.1.0` is their intended provider by high-confidence inference, but exact symbol-provider binding requires that library's export table.
- Verification limits: the earlier `/proc/<PID>/cmdline` rendering replaced NUL separators with spaces, so it does not establish whether `langpicker` and `blankwindow` were included in one quoted `-t` value or were separate trailing arguments. The launcher has no direct imported framebuffer, `ioctl`, E-Ink-update, Cairo-drawing, or Lab126-graphics call, but that scoped negative finding does not apply to `libblanket.so.1.0`, the `screensaver` loader, another loaded module, or downstream libraries. `DT_NEEDED` dependency presence does not establish a call by the launcher, and an X11 event loop does not prove that blanket creates the screensaver window or owns the panel refresh.

## FACT-024 — Screensaver loader resolution occurs beyond the launcher boundary

- Classification: FACT
- Source: OBS-012
- Repository / commit: NOT APPLICABLE for the target binary; pinned-source revisions recorded in OBS-012
- Evidence location: target `blanket` ARM Thumb caller at virtual addresses `0x1d28`–`0x1e0a`, dynamic relocations, imported-symbol table, and strings, distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: local read-only analysis of the copied, stripped `/usr/sbin/blanket`; SHA-256 recorded in OBS-012
- Confidence: high for the launcher call and negative import/string scope; no parser-implementation confidence
- Relevance: identifies the smallest next attribution boundary without broad analysis or a device-side loader experiment.
- Evidence: the launcher's `-t` option stores the `optarg` pointer and, after successful `blanket_init`, passes it to the undefined dynamic import `blanket_event_parse_loader(blanket_context, t_optarg, 0)`. The launcher does not construct a filename or symbol, search a directory, or directly import `dlopen`, `dlsym`, `g_module_open`, or `g_module_symbol`. It has `DT_NEEDED libblanket.so.1.0`, no `DT_RPATH`/`DT_RUNPATH`, and no embedded absolute path for that SONAME.
- Verification limits: the parser's token storage and resolution mechanism are not present in the launcher. At this observation boundary, the `libgmodule-2.0.so.0` dependency was consistent with but did not prove shared-library plugin loading. FACT-026 later resolves that uncertainty using the supplied library.

## FACT-025 — The target `libblanket.so.1.0` is at `/usr/lib/libblanket.so.1.0`

- Classification: FACT
- Source: OBS-013
- Repository / commit: NOT APPLICABLE; direct target-device pathname metadata
- Evidence location: owner-supplied exact-path `ls -l` and `readlink -f` output, distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: Véra/KPM; read-only exact-path metadata query
- Confidence: high for the observed pathname, regular-file mode, owner/group, size, and canonical-path result
- Relevance: identifies the exact next proprietary artifact without a broad filesystem search.
- Evidence: `/usr/lib/libblanket.so.1.0` was reported as a root-owned regular executable file with mode `0755` and size 76,160 bytes. `readlink -f /usr/lib/libblanket.so.1.0` returned `/usr/lib/libblanket.so.1.0`.
- Verification limits: this does not establish a content hash, whether the file was the exact runtime provider in a particular process instance, its exports, or whether it resolves `screensaver` internally versus through another module.

## FACT-026 — Blanket resolves `screensaver` through a shared-object module boundary

- Classification: FACT
- Source: OBS-014
- Repository / commit: NOT APPLICABLE; offline analysis of an owner-supplied target library
- Evidence location: `docs/research/BLANKET_LOADER_RESOLUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: local read-only analysis of copied `/usr/lib/libblanket.so.1.0`; SHA-256 recorded in OBS-014
- Confidence: high for parser queuing, fixed directory/filename resolution, `dlopen`/`dlsym`, module ABI, and common-library helper capability; no confidence that the unsupplied plugin uses a specific display helper
- Relevance: identifies the narrowest current candidate stock presentation module without altering powerd, blanket, the framebuffer, or the target.
- Evidence: `blanket_event_parse_loader` tokenizes and enqueues loader requests; the loader handler calls load/unload routines. The load path opens `/usr/lib/blanket`, converts a loader name to `%s.so`, builds `%s/%s`, calls `dlopen`, and resolves `init`, `deinit`, `lipcCallbacks`, `lipcCallbackNum`, `x11Callback`, and `kiwiCallback`. Thus the established `screensaver` token maps to the exact candidate path `/usr/lib/blanket/screensaver.so`. The common library separately contains demonstrated screensaver-window naming, X11 window creation/map/unmap, Cairo, `/dev/fb0`, and `ioctl` paths.
- Verification limits: at OBS-014, existence and content of the exact plugin remained unobserved. FACT-027 later confirms its installed symlink and canonical path. Common-library capability still does not establish that the plugin creates the visible window, writes framebuffer pixels, submits an E-Ink refresh, or is safe to skip/unload.

## FACT-027 — The blanket `screensaver` plugin resolves to `screensaver.so.1.0`

- Classification: FACT
- Source: OBS-015
- Repository / commit: NOT APPLICABLE; direct target-device pathname metadata
- Evidence location: owner-supplied exact-path `ls -l` and `readlink -f` output, distilled in `docs/research/BLANKET_LOADER_RESOLUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: Véra/KPM; read-only exact-path metadata query
- Confidence: high for the observed symlink and canonical target
- Relevance: identifies the precise stock plugin artifact to inspect without broad target filesystem enumeration or executing the module.
- Evidence: `/usr/lib/blanket/screensaver.so` is a root-owned symbolic link whose relative target is `screensaver.so.1.0`; `readlink -f` resolved it to `/usr/lib/blanket/screensaver.so.1.0`.
- Verification limits: pathname metadata does not establish the canonical target's file size, hash, ELF identity, exported callback implementation, display operations, runtime behavior, or safety as an intervention boundary.

## FACT-028 — The blanket `screensaver` plugin renders content and presents an X11 screensaver window

- Classification: FACT
- Source: OBS-016
- Repository / commit: NOT APPLICABLE; offline analysis of an owner-supplied target plugin
- Evidence location: `docs/research/BLANKET_LOADER_RESOLUTION.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: local read-only analysis of copied `/usr/lib/blanket/screensaver.so.1.0`; SHA-256 recorded in OBS-016
- Confidence: high for the exported ABI, callback table, event-to-presentation control flow, X11/Cairo rendering, and map/unmap actions; no panel-update-submitter confidence
- Relevance: identifies the narrowest demonstrated stock sleep-screen content/window presentation boundary while preserving the distinction between X11 presentation and E-Ink refresh ownership.
- Evidence: the plugin exports the six symbols required by `libblanket`: `init`, `deinit`, `lipcCallbacks`, `lipcCallbackNum`, `x11Callback`, and `kiwiCallback`; `lipcCallbackNum` is 13. Its callback table subscribes to `com.lab126.powerd` events `goingToScreenSaver`, `outOfScreenSaver`, `exitingScreenSaver`, `userShutdown`, and `outOfShutdown`, plus corresponding HAL/authenticator/test sources and a household profile-switch event. `init` obtains the `blanket_screensaver` window through `blanket_image_get_window` and creates a Cairo Xlib surface/context. The `goingToScreenSaver` path reaches prerender/render logic and `blanket_image_window_bringup`; exit-side paths reach `blanket_image_window_teardown`. Embedded function names and directly reached imports show default-screensaver and book-cover preparation/rendering.
- Verification limits: the plugin does not directly import `ioctl`, a framebuffer/HWTCON/MXCFB interface, or an E-Ink update API. It delegates window and image operations to `libblanket`, Cairo, X11, and window-manager utilities. It also contains LIPC event/property, file-open/write, and `/var/local/blanket/screensaver` state paths, so it must not be treated as a side-effect-free window toggle. Static analysis attributes content preparation and X11 window presentation, not physical panel-refresh submission, runtime event ordering in a particular trial, or safety of unloading/skipping the module.

## FACT-029 — Screensaver entry has a narrow X11 map helper inside a broader stateful callback

- Classification: FACT
- Source: OBS-017
- Repository / commit: NOT APPLICABLE; local static analysis of owner-supplied target libraries
- Evidence location: target `screensaver.so.1.0` entry/state-machine code at virtual addresses `0x3680`–`0x4051`, X11 callback beginning at `0x407c`, unmap/exit paths at `0x51e8`–`0x535f`, and deinit at `0x5500`; target `libblanket.so.1.0` bringup/teardown at `0x864c`–`0x894f`, distilled in `docs/research/PRESENTATION_BOUNDARY_SEMANTICS.md`
- Kindle model / firmware: UI-identified Kindle Scribe, generation UNKNOWN / 5.19.5
- Environment: local read-only static analysis; no Kindle interaction and no copied artifact execution or modification
- Confidence: high for direct calls, X11 operations, state stores, callback teardown, and module unload breadth; no runtime-safety or physical-panel confidence
- Relevance: identifies the narrowest semantic presentation boundary and shows why neither a naïve bringup no-op nor whole-module unload yet qualifies as a safe presentation-only StayInk experiment.
- Evidence: the `goingToScreenSaver` callback emits metrics, updates an ASR message argument, selects a mode, performs content/LIPC/Cairo/cache work, then calls `blanket_image_window_bringup` when plugin presentation state is zero. The common helper checks X11 map state, selects input, calls `XMapRaised` and `XFlush`, and records the window; it performs no draw, LIPC, file, win-manager, framebuffer, or ioctl operation. The caller ignores the helper return and immediately stores presentation state 1. Mapping can cause the plugin's X11 Expose path to perform deferred render and flash-trigger work. `outOfScreenSaver` conditionally unmaps based on plugin state, clears state/mode, and sends `unmap_screensaver` plus `nonDtcpScreensaverOut`; `exitingScreenSaver` only emits metrics. Whole-module unload unsubscribes callbacks, invokes deinit, destroys X11/Cairo/content state, closes the shared object, and frees its loader record.
- Verification limits: static evidence does not establish which conditional render branch ran in a particular trial, whether an X11 map causes the physical replacement, whether post-map side effects are required, whether teardown after a skipped map is runtime-safe, or whether any unmodified narrow control exists. The first reversible target experiment remains blocked.
