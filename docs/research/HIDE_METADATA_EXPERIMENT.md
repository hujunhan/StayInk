# Phase 4A — Bounded `HIDE`-metadata experiment design

## Scope and authorization

This document designs one reversible, runtime-only experiment for the
UI-identified Kindle Scribe on firmware 5.19.5. It does not authorize or
execute the experiment. No Kindle command was run, no experiment helper was
implemented, and no Amazon binary, service, framebuffer, HWTCON interface,
powerd property, kernel setting, rootfs file, or startup hook was changed.

The experiment tests HYP-002 only:

> If bare Awesome `HIDE` metadata is added to the persistent stock
> screensaver window before `goingToScreenSaver`, the stock plugin may keep its
> normal lifecycle and map call while Awesome withholds visible presentation.

The hypothesis includes two independent outcomes that must both be observed:

1. the Notebook image remains visible instead of stable stock screensaver
   content; and
2. normal powerd readiness and genuine kernel `mem` suspend still occur.

Neither outcome is assumed.

## Evidence scope

- Generic `HIDE` syntax: SRC-003, KindleModding documentation, commit
  `cf4457c4808636dac5e06d2f2761cb4632efe66d`,
  `content/kindle-dev/awesome-window-manager/__index.md`, section `[HIDE]`,
  CC BY-NC 4.0.
- Target window identity: OBS-022 and OBS-023, supplied read-only observations
  from the UI-identified Scribe on firmware 5.19.5 in the current Véra/KPM
  environment with the stock Amazon UI foregrounded.
- Applicability: the source syntax is generic Kindle community documentation;
  it does not prove Scribe 5.19.5 behavior. The target observations establish
  the serialized grammar and awake state, not `HIDE` semantics.

## Exact single variable

The only presentation variable changed is the value of the selected X11
window's `WM_NAME` property. The selected runtime object from FACT-035 and
FACT-036 is XID `0x400001`, reported PID 4524 (`/usr/sbin/blanket`), layer
`SS`, role `screenSaver`, full-screen, and unmapped in the accepted awake
snapshot.

The experiment must not add or alter `_NET_WM_NAME`, map state, stacking,
geometry, input masks, another X property, a LIPC property, blanket module
state, or any display/power interface.

## Exact `HIDE` serialization

SRC-003 documents Kindle Awesome titles as underscore-delimited metadata
tokens. Its `HIDE` section defines `HIDE` as a bare flag and gives this exact
form:

```text
L:A_N:application_HIDE_ID:org.kindlemodding.hiddenwindow
```

It separately documents `HIDE:background`, whose purpose is not sufficiently
known. This experiment must use bare `HIDE`; `HIDE:background` is excluded.

OBS-022 establishes that the target window uses the same token grammar:

```text
ORIGINAL_TITLE = L:SS_N:screenSaver_O:U_FH:F_module:screensaver_ID:blanket-screensaver_FS:F
```

Following SRC-003's demonstrated placement immediately after the `N` token,
the only permitted candidate mutation is:

```text
HIDE_TITLE = L:SS_N:screenSaver_HIDE_O:U_FH:F_module:screensaver_ID:blanket-screensaver_FS:F
```

The byte-level transformation is exactly one insertion:

```text
...N:screenSaver_O:U...
                  ^
...N:screenSaver_HIDE_O:U...
```

No existing token changes value or position. The insertion is the five ASCII
bytes `_HIDE` before the existing `_O:U` token.

**FACT — generic syntax:** SRC-003 directly documents bare `HIDE` as an
underscore-delimited Awesome title token and says the title can be updated
without it to show the window.

**FACT — target grammar:** OBS-022 directly establishes the target's `L`, `N`,
`O`, `module`, `ID`, `FH`, and `FS` underscore-delimited title form.

**INFERENCE:** The exact `HIDE_TITLE` above is the least-assumptive application
of the documented token to the target title. Confidence is medium: the target
clearly uses the documented grammar, but target Awesome behavior for this flag
has not been exercised.

**UNKNOWN:** Whether Scribe 5.19.5 honors `HIDE` for an `L:SS` client, whether a
hidden mapped client receives Expose, and whether the panel remains unchanged.
Those are experiment outcomes, not syntax assumptions.

## Title-guard mechanism contract

No existing target command is approved for mutation. `xprop` is absent, and
the available KOReader `wmctrl` has not been shown to replace only stock
`WM_NAME`; its broader name/icon-name interfaces are unnecessary. The smallest
acceptable future mechanism is a purpose-built, one-shot Xlib title guard
stored and run only from a dedicated directory under
`/mnt/us/stayink-observation/phase4a-<UTC>/`.

This is a design contract, not implemented code or an executable command.

### Required property operations

The guard must:

1. open the current X display;
2. address only expected XID `0x400001`;
3. read `_NET_WM_PID` with `XGetWindowProperty` and require cardinal PID 4524;
4. read `WM_NAME` with `XGetWindowProperty`, recording its returned type,
   format, byte count, and exact bytes;
5. require type `XA_STRING`, format 8, and exact `ORIGINAL_TITLE` bytes;
   require the returned item count to equal the ASCII literal length and no
   embedded or trailing NUL byte;
6. save the returned original type, format, length, and bytes before mutation;
7. prepare the rollback watchdog and receive its explicit ready acknowledgement;
8. replace only `WM_NAME`, using `XChangeProperty` with `PropModeReplace`, the
   same type and format, and the exact `HIDE_TITLE` byte count without a C
   terminator;
9. call `XSync` and fail on any X error;
10. read `WM_NAME` again and require exact type, format, length, and
    `HIDE_TITLE` bytes;
11. close the X connection rather than keeping an X client active during
    suspend.

The guard must not use `XMapWindow`, `XUnmapWindow`, `XRaiseWindow`,
`XLowerWindow`, `XStoreName`, `_NET_WM_NAME`, an EWMH state request, or any
input/display/power API. Direct `XChangeProperty` is required so the reviewed
operation is one property replacement with a byte-exact saved inverse.

### Saved rollback state

Before mutation, create a new run directory with restrictive permissions and
`O_CREAT|O_EXCL` files. At minimum preserve:

- raw original `WM_NAME` type, format, byte count, and bytes;
- expected XID and owner PID;
- exact expected and modified title hashes;
- mutation UTC, local time, `CLOCK_BOOTTIME`, and `CLOCK_REALTIME` values;
- guard/watchdog status and logs.

The watchdog must also retain the original bytes in its own memory. The file
is recovery evidence, not the source of an assumed/reconstructed title. The
guard must flush and verify the saved record before allowing mutation.

### Exact restore operation

Manual restore and watchdog restore must use the same routine:

1. open a fresh X connection;
2. require XID `0x400001` still exists;
3. require `_NET_WM_PID` is still 4524 and `/proc/4524/exe` still resolves to
   `/usr/sbin/blanket`;
4. read current `WM_NAME` exactly;
5. if it already equals the saved original, perform no write and report
   restored;
6. if it exactly equals `HIDE_TITLE`, replace only `WM_NAME` with
   `XChangeProperty(..., PropModeReplace, ...)` using the saved original type,
   format, item count, and raw bytes;
7. call `XSync`, read back, and require an exact original match;
8. if XID, PID, executable, type, format, or current title differs, refuse to
   overwrite an unknown window/property and enter manual recovery.

Restoration must never reconstruct the baseline from the documented literal.
The literal is a precondition; the saved property bytes are the restore value.

If the original window was destroyed, the per-window mutation disappeared
with it. The guard must not search for and modify another XID automatically.
An operator may run the read-only Phase 3F identity procedure again and verify
that a newly created screensaver window has the stock title.

## Preconditions and abort gate

All preconditions must be collected in one awake session immediately before
arming. Any mismatch aborts without changing `WM_NAME`.

### UI and device condition

- Stock Notebook is visibly foregrounded on a saved, non-sensitive test page.
- No unsaved pen stroke, export, sync operation, dialog, keyboard, or modal UI
  is active.
- Passcode, Wi-Fi, charging, cover, orientation, and Special Offers state are
  recorded; the first trial does not vary them.
- No cover, stylus, RTC, or injected wake path is tested.
- A phone camera is recording the Kindle, physical button, illumination, and
  panel for the entire cycle.

### Power/lifecycle condition

- `com.lab126.powerd state` is exactly `active`.
- `com.lab126.powerd status` shows no defer/suspend grace and enough remaining
  active time to finish preflight; less than 240 seconds aborts.
- The narrow powerd observer has produced no transition event during a
  human-timed ten-second awake control interval.
- The suspend success/failure counters and filtered kernel PM tail are saved.

### X11/ownership condition

- XID `0x400001` exists and direct attributes still show 1860x2480, depth 8,
  `StaticGray`, `InputOutput`, `IsUnMapped`, and override-redirect false.
- `_NET_WM_PID` is exactly 4524.
- PID 4524 remains the current D-Bus owner of `com.lab126.blanket`.
- `/proc/4524/exe` resolves exactly to `/usr/sbin/blanket`.
- `com.lab126.blanket load` includes `screensaver`.
- Raw `WM_NAME` is `XA_STRING`/8 and byte-for-byte `ORIGINAL_TITLE`.
- `isScreenSaverLayerWindowActive`, if responsive on its single query, is
  exactly `0`; another value aborts. Timeout is recorded and does not trigger
  repeated queries.
- No screensaver transition is in progress, established jointly by powerd
  `active`, unmapped X state, layer getter when responsive, and the quiet event
  interval.

### Guard/recovery condition

- The reviewed guard binary and source correspond to an approved hash.
- A no-write dry run has demonstrated every precondition, raw title capture,
  state-file verification, X-error handling, and zero property writes.
- `clock_gettime(CLOCK_BOOTTIME)` and
  `timerfd_create(CLOCK_BOOTTIME, TFD_CLOEXEC)` succeed.
- A short awake-only timer self-test completes and leaves no process behind.
- The detached watchdog reports ready before mutation.
- Manual restore has been rehearsed against a disposable X server, not the
  Kindle stock window.

## Observer and evidence design

The existing narrow observer remains:

```text
lipc-wait-event -m -s 0 -t com.lab126.powerd \
  goingToScreenSaver,outOfScreenSaver,exitingScreenSaver,readyToSuspend,wakeupFromSuspend
```

It is subscription-only under the already-reviewed Phase 2B semantics. It
writes only its log under the run directory. It must be detached from the SSH
session, have no terminal, and be supervised as an experiment child—not a
daemon or startup service. The watchdog terminates only this owned observer
after restoration or at its final deadline.

Observer effects remain confounders: one LIPC subscriber, the guard/watchdog
process, user-storage log writes, and any active SSH/networking can affect
timing. After arming, disconnect SSH if doing so does not terminate the local
children. Do not use network loss as a suspend oracle.

The operator reports that the existing SSH transport resumes after ordinary
wake. The normal restore path may use that resumed connection, but the
fail-safe must not depend on it. After reconnecting, the operator first verifies
that the local event log contains `exitingScreenSaver`, then invokes the
guarded restore. If SSH does not resume, the independent absolute watchdog
remains the bounded fallback.

Phone video is the physical/visual domain. Software logs are correlated using
both UTC and target-local timestamps captured immediately before arming, plus
the guard's mutation timestamps. Millisecond precision is not required. The
operator should film or audibly speak the visible terminal marker and the
physical button press. Passcode entry must remain outside the camera frame or
be redacted; the recording needs only the panel transition before entry and
the restored Notebook after authentication.

### Operation classification

| Proposed operation | Classification | Scope |
| --- | --- | --- |
| PRE/POST getters and reads (`date`, suspend statistics, filtered existing kernel logs, D-Bus owner metadata, selected `/proc` metadata, LIPC getters, and X11 property/attribute reads) | `READ_ONLY` | Reads already reviewed or narrowly scoped metadata; output files under the run directory are experiment records. |
| Narrow `lipc-wait-event` subscription | `READ_ONLY` | Subscribes to the already-approved event set and does not set a property; the extra subscriber and log writes remain observer effects. |
| No-write title-guard dry run and awake timer self-test | `STATE_CHANGING` | Must perform no X property change; its only changes are bounded observation records and short-lived experiment-owned process/timer state. |
| Replace `WM_NAME` with exact `HIDE_TITLE` | `STATE_CHANGING` | The experiment's sole presentation-state mutation. |
| Restore saved raw `WM_NAME` | `STATE_CHANGING` | Required exact inverse and rollback; refuses unexpected identity or title state. |
| Normal physical sleep and wake button presses | `STATE_CHANGING` | Human-initiated stock lifecycle actions, limited to one sleep request and one wake request. |
| Watchdog and owned-observer termination | `STATE_CHANGING` | Changes only experiment-owned process state and the title rollback when required. |

## Exact bounded sequence

### A — Prepare

1. Save and close the non-sensitive Notebook test page normally; reopen it and
   leave it visibly foregrounded.
2. Start phone video, framing the panel, illumination, and physical power
   button.
3. Create a unique Phase 4A run directory under `/mnt/us/stayink-observation/`.
4. Start the detached narrow event observer and verify it is alive.
5. Record UTC and local time together so the target's local event timestamps
   can be related to kernel UTC.

### B — PRE evidence

Record, without writes to system state:

- `date -u` and local `date` markers;
- `/sys/kernel/debug/suspend_stats`;
- filtered existing kernel lines for `PM: suspend entry`, preparation,
  `PM: Suspending system (mem)`, `Suspended for`, wake source, finishing wake,
  and `PM: suspend exit`;
- powerd `state` and `status`;
- current blanket D-Bus owner PID and `/proc/<PID>/exe`;
- blanket loaded-module getter;
- exact X attributes, `_NET_WM_PID`, and raw `WM_NAME`;
- the single screensaver-layer getter result;
- event-observer quiet interval;
- current battery, charging, Wi-Fi, orientation, passcode, cover, and foreground
  UI condition.

### C — Arm and mutate

1. The title guard repeats all mutable preconditions.
2. It saves and verifies the exact original property and rollback record.
3. It starts the detached suspend-aware watchdog and waits for READY.
4. It replaces only `WM_NAME` with exact `HIDE_TITLE`.
5. It synchronizes X11 and reads back an exact modified match.
6. It emits a UTC/local/boottime `HIDE_ARMED` marker.
7. Any failure before exact readback triggers immediate restore and abort.

### D — One normal sleep request

1. The operator physically presses the power button exactly once, promptly
   after the armed marker.
2. Video records:
   - Notebook content visibility;
   - illumination darkening;
   - every black/white flash;
   - any partial or stable screensaver content;
   - frontlight behavior.
3. Do not touch the screen, stylus, cover, USB, Wi-Fi, or another input.
4. Wait at least 180 seconds by the phone/independent wall clock before wake.
   Do not use `/bin/sleep` as evidence or as the deadline mechanism. The target
   baseline takes roughly 115 seconds to kernel entry, so 180 seconds should
   provide observable suspended residency if progression remains normal.

### E — One normal wake

1. Physically press the power button exactly once.
2. Record wake illumination, panel changes, passcode/Notebook restoration, and
   time until normal interaction.
3. Allow `outOfScreenSaver`/`exitingScreenSaver` and normal teardown to finish.
4. Do not draw into the Notebook merely to test responsiveness; use a
   non-destructive navigation action and return to the test page.

### F — Restore before analysis

1. After SSH resumes, verify that the local event log records
   `exitingScreenSaver`, then invoke the guard's restore mode using only the
   saved original bytes. Do not restore earlier merely because the connection
   returned.
2. Require exact original title readback.
3. Require the window is unmapped, reported PID/owner is blanket, and the
   layer getter is 0 if responsive.
4. If another guarded restore request already succeeded, any later restore
   path observes the exact original and exits without writing.

### G — POST evidence

Collect:

- UTC/local POST marker;
- suspend statistics;
- the same filtered kernel PM log set;
- complete narrow powerd event log;
- kernel-reported `Suspended for` duration and wake source;
- current powerd state/status;
- exact X map state, PID, and title;
- screensaver-layer getter;
- watchdog/restore status;
- video-derived sleep and wake observations;
- confirmation that Notebook is visible and normally interactive.

Copy the run directory and video to the development machine before cleanup.
Then remove only the explicitly named Phase 4A helper, state, PID, and log files
from that single run directory. Do not use a broad recursive deletion target.
Verify no observer/guard PID remains and no startup/configuration file was
created.

## Suspend-aware fail-safe

### Watchdog design

The watchdog is a detached, one-shot child prepared before mutation. It:

- owns no terminal and survives SSH disconnect/SIGHUP;
- retains the exact original property bytes in memory and in the run record;
- closes all X connections while waiting;
- creates a one-shot absolute timerfd on `CLOCK_BOOTTIME` for ten minutes after
  mutation;
- holds no wake lock and uses no `_ALARM` clock, RTC, or wake injection;
- at expiry, opens a fresh X connection and runs the exact guarded restore;
- if the title is already original, performs no write;
- exits after logging restore/no-op/failure and terminating only its owned event
  observer.

`CLOCK_BOOTTIME` includes time spent suspended, unlike the previously observed
`/bin/sleep` behavior. A normal timerfd cannot execute userspace code while the
kernel is suspended and this design intentionally does not wake the device.
Therefore:

- if suspend never happens, it restores at the ten-minute deadline;
- if the Kindle wakes before ten minutes and SSH resumes, manual guarded
  restore occurs after verified `exitingScreenSaver`; the watchdog then sees
  the exact original and exits without writing;
- if the deadline passes during suspend, the timer is already expired and the
  watchdog restores as soon as userspace runs after normal resume;
- if the kernel never resumes, no userspace watchdog can restore the property.

This is suspend-inclusive bounded recovery, not a claim that restoration can
execute at a wall-clock instant while the CPU remains suspended.

### Independent manual recovery

Manual recovery does not depend on completing a successful cycle:

1. If SSH/local shell is available, invoke guarded restore immediately.
2. If the device is asleep, use one physical power press, wait for userspace,
   then use resumed SSH for guarded restore after normal teardown; if SSH does
   not resume, let the watchdog deadline perform the restore.
3. If the XID disappeared, do not write another XID. Re-run read-only identity;
   destruction normally removes the mutated property with the old window.
4. If title restoration succeeds but UI remains inconsistent, wait for ordinary
   stock teardown once, then re-query state.
5. If the UI remains softlocked after watchdog/manual restore, a physical
   long-hold restart is the last-resort recovery. This may lose unsaved state,
   which is why the trial requires a saved, non-sensitive page and no pending
   edit. Window recreation should discard the runtime-only property, but that
   must be verified after restart rather than assumed.

No fail-safe may restart an Amazon service, write rootfs, inject input, modify
powerd, or schedule an RTC wake.

## Failure classification and immediate rollback

| Failure | Evidence | Immediate action |
| --- | --- | --- |
| HIDE ignored | Stable stock screensaver appears | If the device otherwise behaves normally, complete one wake, run guarded restore immediately, and stop. Do not repeat. |
| Partial presentation | Flash, partial image, or repaint occurs without stable stock content | Record video; complete normal wake if responsive; restore immediately; classify separately from success even if Notebook later reappears. |
| Lifecycle regression | Readiness stalls materially, suspend success does not increment, kernel entry is absent, or a suspend failure increments | At 240 seconds by independent clock, use one physical wake/cancel press, restore as soon as userspace is available, and stop. Do not alter powerd to force progress. |
| Wake regression | Physical wake does not restore normal UI | Wait for the suspend-aware watchdog, attempt manual guarded restore if a shell is available, then use last-resort physical restart only after the restore deadline/recovery checks. |
| Title restoration failure | Readback is not exact original | Do not overwrite an unexpected title. Preserve logs, verify XID/PID, retry guarded restore only if current title is exact `HIDE_TITLE`; otherwise use manual identity/restart recovery. |
| X11/Awesome inconsistency | Title is original but map/layer/focus state is unexpected | Stop input, record state, allow one normal teardown interval, re-query, and use physical restart if the UI remains unusable. Do not map/unmap/lower the window manually. |
| UI softlock | UI does not respond after title restore and normal wake interval | Preserve video/logs; allow watchdog; last-resort physical restart. Do not restart/kill Awesome, blanket, pillow, or KPPMainApp. |

Any failure ends the experiment. There is no same-session retry.

## Success criteria

Success requires every item below. A mixed result is a failure or partial
presentation result, not success.

1. The starting Notebook content remains visible through stock sleep entry,
   apart from separately described illumination or transient panel behavior.
2. No stable stock screensaver content is visibly presented.
3. `goingToScreenSaver`, repeated `readyToSuspend`, resume,
   `wakeupFromSuspend`, `outOfScreenSaver`, and `exitingScreenSaver` retain the
   established order without a material readiness stall.
4. Readiness progression is comparable to Baseline B, including transition to
   kernel entry rather than indefinite repeated readiness.
5. The kernel logs a complete `mem` suspend transaction and suspend success
   increments exactly once with failure counters unchanged.
6. Kernel `Suspended for` independently records nonzero residency consistent
   with the physical wait; powerd terminology alone is insufficient.
7. One physical wake succeeds normally.
8. Stock Notebook returns and is normally interactive without recovery.
9. The screensaver window returns to `IsUnMapped`; the layer getter is 0 if
   responsive.
10. `WM_NAME` type, format, length, and bytes exactly match the saved original.
11. No helper/observer process, altered X property, device configuration,
    startup hook, or rootfs change remains.

## Abort conditions before mutation

Abort without changing `WM_NAME` if any of these occurs:

- pinned `HIDE` syntax evidence is contradicted or the title is not the exact
  expected target grammar;
- XID, PID, D-Bus owner, executable, geometry, depth, visual, map state,
  override-redirect, module list, powerd state, or title differs;
- layer getter responds with a value other than 0;
- a powerd transition event appears during preflight;
- raw original property cannot be saved and verified exactly;
- the proposed helper writes `_NET_WM_NAME` or any property besides `WM_NAME`;
- the helper can map, unmap, restack, draw, call LIPC setters, or touch power or
  display interfaces;
- the watchdog cannot prove `CLOCK_BOOTTIME`/timerfd support and readiness;
- detached-process, SIGHUP, exact-restore, X-error, or cleanup behavior has not
  passed review;
- phone video, independent time source, event observer, manual operator, or
  last-resort physical recovery is unavailable;
- any Amazon service would need to be stopped/restarted or any persistent hook
  would be required.

## Authorization recommendation

The experiment is conceptually bounded to one property and has a credible
rollback architecture. The read-only target evidence is sufficient to finish
this design.

Execution should **not yet be authorized**. Phase 4B now provides reviewed
source and passing host-core tests, but no audited ARM/Linux no-write binary or
target dry run exists, and target `CLOCK_BOOTTIME`/timerfd readiness has not
been proven. Using `wmctrl` as a shortcut would broaden or obscure the mutation
and does not satisfy this design.

Authorization may be reconsidered only after the remaining Phase 4B review
shows that the helper:

- contains no operation beyond exact reads, one guarded `WM_NAME` replacement,
  its exact inverse, logs, and its owned timer/child cleanup;
- preserves the already-passing host mismatch/compare-before-write tests and
  passes disposable-X restore tests;
- passes a target no-write dry run and awake-only timer self-test;
- leaves no process behind after the dry run;
- has exact manual invocation, rollback, and cleanup commands reviewed before
  the title-changing trial.
