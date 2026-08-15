# Candidate Mechanisms

These are untested research hypotheses, not designs. Both leave the sleep request and powerd suspend sequence intact in principle. Neither is approved for a device experiment yet.

## Ranking

| Rank | Candidate | Safety | Simplicity | Evidence | Genuine-suspend compatibility |
| --- | --- | --- | --- | --- | --- |
| 1 | restore the pre-sleep panel image in the observable pre-suspend window | medium | medium | medium | likely, unverified |
| 2 | suppress only the blanket screensaver module for the transition | low–medium | medium | low–medium | plausible, unverified |

Candidate 1 ranks first because it can be one-shot and avoids changing stock component lifecycle. It still requires a direct MTK display update, so it is not low risk. Candidate 2 uses a control demonstrated by KOReader but could disturb stock UI state and wake restoration.

## HYP-001 — One-shot framebuffer restoration before suspend

- Classification: INFERENCE
- Kindle model / firmware: Scribe / UNKNOWN
- Confidence: medium-low
- Status: untested

1. **Mechanism:** After a user sleep request and the unwanted stock replacement, restore an in-memory copy of the pre-sleep framebuffer and submit one correct full-panel MTK refresh before the final readiness-to-suspend boundary. Do not alter powerd state or timeouts.
2. **Supporting evidence:** FACT-003 provides an observable interval between screen-saver entry and readiness on one Scribe; FACT-015 confirms repeated readiness events followed by kernel `mem` suspend on the target with KOReader active; FACT-005 shows memory updates and panel refresh are separate; FACT-006 shows Scribe-specific MTK support in two projects; FACT-010 makes passive persistence plausible.
3. **Assumptions:** the framebuffer contains a usable copy before replacement; replacement completion can be detected; the restore can finish before suspend; no later stock refresh overwrites it; one display update does not delay powerd.
4. **Unknowns:** actual render timing, buffer layout across rotation/pen modes, whether suspend or resume reconstructs display state, correct safe waveform, and whether the observed interval exists on target firmware.
5. **Risk of preventing real suspend:** medium. A race, long update, open device handle, or accidental timeout interaction could delay readiness even without intentionally inhibiting it.
6. **Risk of interfering with wake:** medium. FACT-011 shows long-sleep Scribe wake refreshes already have failure modes in KOReader; a stale buffer or competing wake refresh could corrupt or obscure the UI.
7. **Risk of persistent system-state modification:** low if the image remains in RAM and no files/properties are changed. Reading pixels still has privacy implications.
8. **Smallest safe test:** Baseline A established the KOReader software/kernel order but did not synchronize the visible replacement to events or observe the stock renderer. Before any state-changing test, complete a separately reviewed stock-UI control with shared visual/software synchronization. Only later, with explicit approval and a non-sensitive test page, run a one-shot process from user storage that restores once, exits before suspend, and records external power plus normal wake. Success requires the original image to remain visible, measured low-power behavior matching control, and normal stock UI after wake.
9. **Falsified by:** replacement completes after the last safe execution point; the visible panel cannot be reconstructed from the captured buffer; a later firmware refresh overwrites it; suspend power or timing differs materially from control; or wake requires recovery.
10. **Read-only / reversible:** the observation phase is read-only. The decisive test writes only display state and is runtime-only but not read-only. Rollback is ordinary wake/full redraw; the trial must not run unless that recovery is verified first.

## HYP-002 — Event-scoped blanket screensaver suppression

- Classification: INFERENCE
- Kindle model / firmware: Scribe / UNKNOWN
- Confidence: low
- Status: untested

1. **Mechanism:** Temporarily suppress only the native blanket `screensaver` presentation around a sleep transition, then restore that module on wake, without writing any powerd property other than the user's normal request.
2. **Supporting evidence:** FACT-013 maps `com.lab126.blanket` to `/usr/sbin/blanket` on the target Scribe and records a literal `screensaver` command-line argument; FACT-012 shows KOReader can unload/reload a blanket screensaver module in a conditional launch mode; FACT-003 and FACT-015 separate screen-saver entry from readiness in KOReader environments; SRC-009 reports lock continuing without visible replacement when modified stock resources failed to resolve.
3. **Assumptions:** blanket owns or gates the replacement rather than merely hosting a named module; its runtime control does not gate readiness; unloading is transient; reload is sufficient for normal wake; Special Offers/passcode variants do not use a different actor.
4. **Unknowns:** rendering and refresh ownership, accepted values, internal dependencies, timing, persistence, and behavior across firmware and Scribe generations.
5. **Risk of preventing real suspend:** medium. The module may participate in acknowledgements or state transitions even though KOReader treats it as display-side.
6. **Risk of interfering with wake:** high. Failure to reload or a firmware-specific dependency could leave the lock screen or normal UI unavailable.
7. **Risk of persistent system-state modification:** low only if LIPC load state is truly runtime-only; that is unverified. No files or init configuration may be changed.
8. **Smallest safe test:** Phase 2A confirmed the target publisher, executable, and `screensaver` argument; Baseline A confirmed KOReader event/kernel ordering but did not correlate blanket with the visible replacement. A separately reviewed stock-UI trace must establish that correlation before any state-changing trial is designed. A later approved trial would change only the runtime module state for one non-sensitive button cycle, with an independent timer/operator recovery and reboot already validated. Success additionally requires control-equivalent low-power measurement and full normal UI restoration.
9. **Falsified by:** the module is absent or not the renderer; unload suppresses readiness or low-power entry; another process redraws the screen; reload fails to restore stock behavior; or any effect persists after reboot.
10. **Read-only / reversible:** discovery can be read-only. The decisive LIPC operation is state-changing. It is only a candidate if target-device observation first shows a complete recovery path; no persistent fallback is acceptable.

## Excluded directions

- `preventScreenSaver`, `deferSuspend`, `abortSuspend`, and disabling suspend are not candidates because their semantics are unverified and they may violate the genuine-suspend invariant.
- Rootfs screensaver-image changes are excluded despite SRC-009 because they are persistent, unsafe, and do not prove suspend.
- Process injection or refresh-ioctl interception is excluded because current evidence does not justify that complexity or risk.
