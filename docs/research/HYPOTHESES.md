# Candidate Mechanisms

These are untested research hypotheses, not designs. Both leave the sleep request and powerd suspend sequence intact in principle. Neither is approved for a device experiment yet. Phase 3F prioritizes the non-patching presentation-only branch; framebuffer restoration is retained only as historical research context and is not the current investigation direction.

## Ranking

| Rank | Candidate | Safety | Simplicity | Evidence | Genuine-suspend compatibility |
| --- | --- | --- | --- | --- | --- |
| 1 | preserve the blanket lifecycle but request that Awesome hide its persistent screensaver window before stock map | low–medium | medium | medium-low | plausible, unverified |
| 2 | restore the pre-sleep panel image in the observable pre-suspend window | medium | medium | medium | likely, unverified; deferred |

Candidate 1 ranks first because it is the highest-level non-patching boundary currently known, preserves the stock plugin lifecycle and internal presented state in principle, and can be staged before the post-map Expose race. Target Awesome behavior and rollback remain unverified. Candidate 2 requires a direct MTK display update and is deferred by the Phase 3F scope.

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
- Status: read-only identity gate passed on the observed Scribe 5.19.5; reversible runtime trial not yet designed or approved

1. **Mechanism:** Keep the native blanket `screensaver` module and its lifecycle callback loaded. If target read-only evidence validates the exact persistent window metadata and Awesome's title convention, temporarily add `HIDE` to an otherwise byte-equivalent title before stock map, then restore the byte-exact original title on wake. Do not write a powerd property or alter the plugin's own presentation state.
2. **Supporting evidence:** FACT-013 maps `com.lab126.blanket` to `/usr/sbin/blanket` on the target Scribe and records a literal `screensaver` command-line argument; FACT-024 through FACT-027 establish the parser, fixed plugin path, dynamic ABI, and canonical file; FACT-028 and FACT-029 establish plugin lifecycle/content work and the narrow X11 map boundary; FACT-030 establishes the statically retained screensaver window; FACT-035 resolves its target runtime identity as an unmapped full-screen `L:SS` / `N:screenSaver` client with exact title metadata; SRC-003 documents a generic Kindle Awesome `HIDE` title flag; FACT-031 shows why a pre-map control is preferable to reacting after map; FACT-003 and FACT-015 separate screen-saver entry from readiness in KOReader environments; SRC-009 reports lock continuing without visible replacement when modified stock resources failed to resolve.
3. **Assumptions:** Scribe 5.19.5 honors `HIDE` on this window; the exact original title can be restored; a hidden-but-mapped screensaver window leaves plugin state coherent; it avoids visible Expose/flash work or otherwise preserves the Notebook; suppression does not gate readiness; recovery is sufficient for normal wake; Special Offers/passcode variants do not use a different actor.
4. **Unknowns:** target Awesome behavior after adding/removing `HIDE`, whether hidden mapping receives Expose/flash work, XID persistence, physical panel result, timing, restoration, wake behavior, and applicability across firmware and Scribe generations.
5. **Risk of preventing real suspend:** medium. The module may participate in acknowledgements or state transitions even though KOReader treats it as display-side.
6. **Risk of interfering with wake:** high. Inconsistent plugin/window state or failure to restore normal visibility could leave the lock screen or normal UI unavailable.
7. **Risk of persistent system-state modification:** low only if a future presentation control is truly runtime-only. The stock plugin may still update its ordinary content-rotation/cache state. No init configuration, rootfs file, or stock cache behavior may be modified by the control itself.
8. **Smallest safe test:** The read-only identity gate in `docs/research/NON_PATCHING_PRESENTATION_CONTROL.md` has passed. The next step is a separate design review—not execution—for one bounded title-metadata trial on a non-sensitive Notebook page. It may change only the title metadata, must have exact and timed restore plus independent operator recovery, and must preserve ordinary powerd lifecycle. Success additionally requires the original Notebook image to remain visible, control-equivalent suspend statistics/kernel PM evidence, normal physical wake, and full Notebook restoration.
9. **Falsified by:** withholding map/layer visibility suppresses readiness or low-power entry; another component still replaces the panel; required post-map side effects cannot be preserved; normal visibility cannot be restored; or any effect persists after reboot.
10. **Read-only / reversible:** discovery can be read-only. Loader `unload`/`load` and any X11 visibility operation are state-changing. Whole-module unload is now classified as too broad for a presentation-only test; no persistent fallback is acceptable.

## Excluded directions

- `preventScreenSaver`, `deferSuspend`, `abortSuspend`, and disabling suspend are not candidates because their semantics are unverified and they may violate the genuine-suspend invariant.
- Rootfs screensaver-image changes are excluded despite SRC-009 because they are persistent, unsafe, and do not prove suspend.
- Process injection or refresh-ioctl interception is excluded because current evidence does not justify that complexity or risk.
