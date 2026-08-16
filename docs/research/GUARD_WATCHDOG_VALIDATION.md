# Phase 4B — HIDE guard/watchdog implementation and no-write validation

## Scope and authorization

Phase 4B implements and audits the guard needed by the accepted Phase 4A
design. It does not authorize `HIDE`, a physical sleep request, or any target
`WM_NAME` mutation. No Kindle interaction occurred during this implementation
work.

Target scope remains the UI-identified Kindle Scribe on firmware 5.19.5 in the
current Véra/KPM environment. OBS-022 and OBS-023 establish the accepted
window identity snapshot. SRC-003 at KindleModding commit
`cf4457c4808636dac5e06d2f2761cb4632efe66d` establishes generic `HIDE` title
syntax, not target runtime behavior.

## Concrete question and success condition

The concrete question is whether one small native program can make the future
transaction structurally compare-before-write, arm a suspend-inclusive
watchdog before APPLY, and provide target dry-run binaries with no X property
writer at all.

Phase 4B host success requires passing pure transaction/state tests and source
audit. Target readiness requires two separately reviewed no-write runs. The
real experiment remains unauthorized until both target results exist.

## Implementation

The source is under `tools/hide-guard/`:

1. `hide_guard_core` contains fixed original/hidden byte strings, raw-property
   comparisons, identity predicates, transaction executors, absolute-deadline
   arithmetic, and the watchdog state machine.
2. `stayink-hide-guard` is the default Linux/Xlib executable compiled with
   `STAYINK_ENABLE_WRITES=0`.
3. `stayink-hide-guard-write-enabled` is separately named and is generated
   only by an explicit future build option. Its source contains one
   `XChangeProperty` call site, gated behind the compile definition.

The program contains no LIPC/DBus client, Kindle power code, framebuffer code,
display-update ioctl, input code, service control, startup integration, or
filesystem search. D-Bus ownership remains a separately reviewed external
precondition.

## Transaction invariants

### APPLY

The common transaction executor calls its writer callback only when the
current property is exactly:

- type: current server's `XA_STRING` atom;
- format: 8;
- item count: `strlen(HG_ORIGINAL_TITLE)`;
- bytes: exact `HG_ORIGINAL_TITLE`, without a trailing NUL.

Every mismatch returns `HG_TRANSACTION_REFUSED` without invoking the writer.
The replacement inherits the captured type/format and uses exactly
`strlen(HG_HIDE_TITLE)` items.

### RESTORE

The executor calls its writer callback only when current `WM_NAME` is exact
`HG_HIDE_TITLE` and the saved original is independently exact `XA_STRING`/8
`HG_ORIGINAL_TITLE`. It passes the saved original property object to the
writer; restoration is not reconstructed.

Current ORIGINAL returns a no-write result. Any other title, type, format,
item count, identity, X error, state-file error, or owner mismatch refuses the
write.

## Identity validation

Every capture validates:

- caller-supplied expected XID and PID;
- successful `XGetWindowAttributes`;
- 1860x2480 geometry, depth 8, StaticGray visual;
- `IsUnmapped` and non-override-redirect state;
- `_NET_WM_PID` with type CARDINAL/32 and one item;
- exact `/proc/<PID>/exe` target `/usr/sbin/blanket`;
- complete `WM_NAME` retrieval with no trailing unread bytes.

The tool never searches for a substitute XID. If the resource disappears or
is reused with mismatched identity, it refuses to write. The current D-Bus
owner of `com.lab126.blanket` must be checked outside the program immediately
before a future transaction.

## Watchdog

The watchdog is forked only after an exact original capture. It retains those
raw bytes through fork copy-on-write, calls `setsid`, ignores `SIGHUP`, drops
terminal descriptors, and creates a `CLOCK_BOOTTIME` (not `_ALARM`) timerfd.
It sends the parent `ARMED` only after `timerfd_settime` succeeds with
`TFD_TIMER_ABSTIME` and the log is flushed.

At expiry it opens a new X connection, repeats full identity validation, and
evaluates RESTORE. ORIGINAL produces `NO_WRITE`. A dry-run build cannot write
even if it unexpectedly observes HIDE because `XChangeProperty` is absent from
the object/binary. A future write-enabled watchdog can restore only exact HIDE
to its retained exact original.

The watchdog does not hold a wake lock and cannot execute while the kernel or
userspace is not running. If its deadline passes during suspend, it can act
only after normal resume. It cannot recover from a kernel/userspace failure
that prevents resume.

## Host validation performed

### FACT — pure tests

On the 2026-08-15 arm64 macOS development host, the C core built with
AppleClang using `-Wall -Wextra -Werror -pedantic`, and all CTest cases passed.
The same tests also passed an AddressSanitizer/UndefinedBehaviorSanitizer
debug build.
The cases cover:

- exact ORIGINAL-to-HIDE bytes;
- exact captured-original restoration through a fake writer;
- X item counts excluding C terminators;
- unexpected title/type/format refusal;
- wrong PID, executable, or override-redirect refusal;
- APPLY and RESTORE compare-before-write callback counts;
- watchdog transition ordering;
- absolute deadline arithmetic and overflow refusal.

### FACT — source/object audit

In the reviewed Phase 4B source there is one `XChangeProperty` call site,
inside `#if STAYINK_ENABLE_WRITES`. Compatibility-header syntax compilation of
both branches completed without compiler warnings. The write-disabled object
did not import `XChangeProperty`; the write-enabled object imported it once.
Neither object imported `XMapWindow`, `XUnmapWindow`, `XRaiseWindow`,
`XLowerWindow`, or `XStoreName`.

This object check is development-host evidence, not a substitute for auditing
the eventual ARM/Linux binary.

## Mutation-capable operations audit

| Operation | Reachability | Effect |
| --- | --- | --- |
| `XChangeProperty` | Compile-gated write-enabled build only | Replaces `WM_NAME`; the only X server mutation. |
| `open(O_CREAT|O_EXCL)`, `write`, `fsync` | Watchdog log; write-enabled saved-state record | Creates bounded experiment diagnostics/state at explicit paths. |
| `fork`, `setsid`, `signal`, `dup2` | Watchdog modes | Transient local process/session/signal/descriptor state. |
| `timerfd_create`, `timerfd_settime` | Timer probe and watchdog | Transient non-alarm timer state; does not wake the Kindle. |
| `XOpenDisplay`/`XCloseDisplay` | All target modes | Creates/removes a transient X client connection. |
| `XSetErrorHandler` | All target modes | Changes only this process's Xlib handler. |
| `XInternAtom(..., True)` | Identity reads | Lookup only; `True` prevents atom creation. |
| `XGetWindowAttributes`, `XGetWindowProperty(..., False)`, `readlink` | Identity reads | Read-only metadata; property deletion is false. |
| `XSync` | X error synchronization | Flushes prior requests; dry-run paths issue no mutating X request. |

No `unlink` or automatic cleanup is present. Evidence must be copied first;
cleanup remains a separately reviewed exact-path manual action.

## Proof of no-write dry-run reachability

The default executable is built with `STAYINK_ENABLE_WRITES=0`:

- the sole `XChangeProperty` call and writer adapter are removed by the C
  preprocessor;
- APPLY and RESTORE modes print `REFUSED` during argument parsing;
- `--dry-run` calls capture, comparison, transformation, and timer probe only;
- `--watchdog-dry-run` passes `writes_allowed=false` and the resulting object
  has no `XChangeProperty` import;
- the host audit script fails if a no-write binary imports that symbol or any
  prohibited map/unmap/raise/lower/name helper.

The eventual ARM no-write binary must pass the same symbol audit and hash
verification before transfer.

## Phase 4B-2 target no-write dry-run design

This sequence is not yet executed. Copy only the audited write-disabled binary
to a unique directory under `/mnt/us/stayink-observation/`. Do not copy the
write-enabled variant.

Before execution, manually confirm the current D-Bus owner PID with the
already-reviewed `org.freedesktop.DBus.GetConnectionUnixProcessID` query and
record that PID. The examples below use the accepted snapshot values; any
change requires human review rather than automatic substitution.

```sh
RUN=/mnt/us/stayink-observation/phase4b-no-write
GUARD="$RUN/stayink-hide-guard"

dbus-send --system --print-reply --dest=org.freedesktop.DBus / \
  org.freedesktop.DBus.GetConnectionUnixProcessID \
  string:"com.lab126.blanket"
readlink -f /proc/4524/exe
"$GUARD" --dry-run --xid 0x400001 --pid 4524
```

Expected result is `DRY_RUN_OK`, exact XA_STRING/8 metadata, the expected item
counts, `CLOCK_BOOTTIME_TIMERFD_SUPPORTED yes`, and `NO_WRITE dry-run-build`.
Any refusal is a safe failure and stops Phase 4B. The command must not be run
during a screensaver transition.

The D-Bus owner query and `readlink` are `READ_ONLY`. Executing the guard is
`STATE_CHANGING`, limited to transient X connection and timerfd runtime state.
It performs no X property write and creates no file.

## Phase 4B-3 detached watchdog no-write design

Preconditions are the successful ordinary dry run, sufficient active-state
time, original unmapped window state, and a nonexistent exact log path.

```sh
RUN=/mnt/us/stayink-observation/phase4b-no-write
"$RUN/stayink-hide-guard" --watchdog-dry-run 15 \
  --xid 0x400001 --pid 4524 --log "$RUN/watchdog.log"
```

The parent must print `WATCHDOG_DRY_RUN_ARMED`. The operator then disconnects
SSH without sending a signal to the child, waits more than 15 seconds while
keeping the Kindle awake, reconnects, and records:

```sh
cat "$RUN/watchdog.log"
"$RUN/stayink-hide-guard" --dry-run --xid 0x400001 --pid 4524
```

Required log ending: `NO_WRITE current=ORIGINAL`. The recorded watchdog PID
must no longer exist under `/proc`, and the second dry run must still report
exact ORIGINAL. Do not trigger sleep in this validation.

Safety classification: `STATE_CHANGING`, limited to a detached transient
process/session, non-alarm timerfd, X connections, and the exclusive diagnostic
log under `/mnt/us`. The audited binary has no X property mutation symbol.

## Results and remaining gate

- Host core tests: **FACT — PASS** on the stated development host.
- Host source/object audit: **FACT — PASS** within the reviewed source and
  compatibility-compiled objects.
- ARM/Linux binary build and symbol/hash audit: **UNKNOWN — not yet performed**.
- Target ordinary no-write dry run: **UNKNOWN — not executed**.
- Target watchdog detachment/expiry no-write run: **UNKNOWN — not executed**.
- Target `CLOCK_BOOTTIME` readiness: **UNKNOWN** until the ordinary dry run;
  expiry behavior remains UNKNOWN until the watchdog dry run.

The code is not yet safe enough to request authorization for real HIDE. First
produce and audit the ARM no-write binary, then separately authorize and run
the two no-write target validations above. Only their successful evidence can
open review of a separately hashed write-enabled binary and the Phase 4A
mutation procedure.

## Remaining failure modes

- Target Xlib/timerfd ABI or cross-build incompatibility.
- XID/PID changes or resource recreation between external and internal checks.
- `_NET_WM_PID` is metadata rather than cryptographic ownership proof.
- D-Bus owner drift after the external query.
- Window still mapped when the expiry-side guard requires `IsUnmapped`.
- X server unavailable at deadline, unexpected title, corrupt state record, or
  user-storage/log `fsync` failure; all cause refusal rather than a write.
- Manual RESTORE and deadline RESTORE could approach each other in time; both
  compare before writing, but no cross-process restoration lock is yet proven
  on the target filesystem.
- `SIGKILL`, OOM termination, process-table failure, or storage failure can
  remove or impair the watchdog; only reboot/X-resource destruction then
  remains as a recovery boundary.
- A watchdog cannot recover until kernel/userspace and X resume normally.
- The no-write tests cannot establish that target Awesome honors `HIDE` or that
  physical panel contents will remain unchanged.
