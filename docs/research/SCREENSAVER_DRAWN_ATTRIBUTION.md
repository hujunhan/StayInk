# Phase 3B — `g_is_screensaver_drawn` attribution

## Status and question

Phase 3B is research-only. The device owner executed the reviewed Stage 1 and
Stage 2 read-only static string inspections and reached the Stage 3 tool
availability gate. The owner later supplied local copies of powerd and
KPPMainAppV2 for offline static analysis (OBS-010). No target binary, service,
LIPC property, tracing state, framebuffer, or power state was changed by these
observations, and neither copied executable was run locally.

The target remains the UI-identified Kindle Scribe running firmware 5.19.5;
its generation and physical model number are **UNKNOWN**. It is jailbroken with
Véra and KPM, and KOReader has been present, so it is not a proven pure-stock
boot.

The concrete question is: what changes the value logged by powerd as
`g_is_screensaver_drawn`, what does the value represent, and does it expose a
safe presentation boundary that could be controlled without changing genuine
suspend?

An answer requires either a target-applicable implementation reference or
narrow static evidence identifying the field's setter/callback and its
consumer. The name and a single observed value are not enough.

## Source search

The exact forms `g_is_screensaver_drawn`, `is_screensaver_drawn`,
`screensaver_drawn`, and natural-language variants of “screensaver drawn” were
searched in the current research corpus and pinned SRC-001 through SRC-006
checkouts. The exact identifier was found only in the target observation
records derived from OBS-006; it was not found in the reviewed pinned-source
scope.

This is a scoped negative result. The stock `/usr/bin/powerd`,
`/usr/sbin/blanket`, `/app/bin/KPPMainAppV2`, and `/usr/bin/awesome` binaries are
not present in the pinned source repositories, so the search cannot establish
that the identifier is absent from the target binaries.

OBS-007 subsequently found the exact identifier and related diagnostics in the
target `/usr/bin/powerd` binary. This is target binary evidence, not public
source and not a call graph.

OBS-010 then traced those diagnostics and the KPP handler in owner-supplied
copies of the target binaries. That local evidence establishes the setter and
named-handler relationships described below; it remains static evidence from
one firmware image, not a runtime trace or stock source release.

Relevant nearby evidence is limited to:

- **FACT — target, high confidence (OBS-006 / FACT-018):** powerd logged the
  literal field `g_is_screensaver_drawn = (1)` while moving from `SUSPENDED` to
  `SCREEN SAVER` on a wake-side transition. It then moved to `ACTIVE` and sent
  `outOfScreenSaver`. The line reports the value; it does not show the value
  changing or identify its setter.
- **FACT — generic inventory, medium confidence (SRC-003):** the generic
  `com.lab126.winmgr` inventory lists read-only
  `isScreenSaverLayerWindowActive`, with no semantic description
  (`content/kindle-apps-and-services/com.lab126.winmgr.md:8-41`). The one target
  query attempted in Phase 2A timed out while awake.
- **FACT — generic advertising-module inventory, medium confidence
  (SRC-003):** `com.lab126.blanket.ad_screensaver` lists read-only
  `screensaver_showing`, again with a `TODO` description
  (`com.lab126.blanket.ad_screensaver.md:7-14`). The target has no Special
  Offers, and target availability of this publisher/property is **UNKNOWN**.
- **FACT — generic window documentation, medium confidence (SRC-003):** Awesome
  window metadata includes an `SS` layer above the application layer and roles
  named `activeSS` and `screenSaver`; the role descriptions are empty
  (`content/kindle-dev/awesome-window-manager/__index.md:35-74`).
- **FACT — third-party behavior only (SRC-004):** KOReader listens for
  `goingToScreenSaver`, `readyToSuspend`, `wakeupFromSuspend`,
  `outOfScreenSaver`, and `exitingScreenSaver`, and separately renders its own
  screensaver. No reviewed KOReader code reads or writes
  `g_is_screensaver_drawn` (`frontend/device/kindle/device.lua:746-851`; SRC-006
  `input/input-kindle.h:143-203`).

None of these similarly named properties or roles is proven to back powerd's
field. In particular, read-only status properties may be observables without
being the callback or state source powerd uses.

## Candidate ownership model

### A — powerd owns and changes internal state

- **Supporting FACTS:** local static analysis identifies three stores to the
  same powerd global at virtual address `0x65814`. One path sends the
  `goingToScreenSaver` LIPC event, then stores integer 1 and logs the value as
  1. Two other powerd paths store 0, including the path with the explicit
  `screensaver_drawn is getting set to false` diagnostic (OBS-010 / FACT-021).
- **FACT:** the setter does not accept a presentation-completion value from
  blanket, Awesome, or KPPMainApp. The assignment is performed by powerd.
- **INFERENCE:** the name most likely denotes powerd transition bookkeeping
  used by its low-temperature/frontlight logic, rather than physical draw
  completion. Confidence is **medium-high**, based on the internal consumer
  path and its frontlight diagnostic.
- **UNKNOWN:** why the value is called “drawn,” which exact state transitions
  reach each store, and whether a synchronous side effect of the preceding
  LIPC send matters to the intended meaning.

### B — blanket reports screensaver readiness to powerd

- **Supporting FACTS:** blanket runs with a literal `screensaver` argument on
  the target; generic evidence exposes blanket module load state; the generic
  advertising module has a read-only `screensaver_showing` property. The target
  powerd binary also contains the configuration-key strings `BLANKET_NAME` and
  `BLANKET_LOAD` (FACT-019). Local analysis resolves the argument as a loader
  name and shows that the launcher delegates loader and event behavior to
  `libblanket.so.1.0` (OBS-011 / FACT-023).
- **INFERENCE:** powerd appears aware of blanket configuration or load state,
  making blanket a somewhat stronger external-trigger candidate. String
  presence does not identify a callback or completion report.
- **Confidence:** **low-to-medium** for some powerd/blanket relationship; low
  for draw-completion reporting.
- **UNKNOWN resolution:** an exact completion interface present in both the
  blanket shared-library/loader boundary and powerd, or a target-applicable
  call/configuration path connecting them. The copied launcher exposes no such
  report.

### C — winmgr/Awesome reports screensaver-layer visibility

- **Supporting FACTS:** Awesome owns `com.lab126.winmgr` on the target; generic
  evidence defines the SS layer and read-only
  `isScreenSaverLayerWindowActive`.
- **INFERENCE:** layer visibility is conceptually compatible with a logical
  “drawn” flag, but no reviewed evidence equates the two or shows Awesome
  reporting to powerd.
- **Confidence:** **low**.
- **UNKNOWN resolution:** matching callback/property names in both binaries or
  a target log showing the layer state change and powerd field change in a
  shared transaction.

### D — KPPMainApp reports presentation completion

- **Supporting FACTS:** KPPMainApp owns the named `ScreenSaverListener`
  publisher and reacts to screensaver stop on the target. Local static analysis
  shows that `ApplicationModule::registerScreenSaverListener()` maps both
  powerd `goingToScreenSaver` and `outOfScreenSaver` to the same
  `ApplicationModule::screenSaverHandler()` (OBS-010 / FACT-022).
- **FACT:** that handler toggles object-local metrics state, measures elapsed
  time, and calls `emitDeviceEngagementMetrics`. It is not the setter of
  powerd's global and contains no demonstrated presentation-completion action.
- **INFERENCE:** the named `ApplicationModule` handler is a lifecycle-metrics
  consumer, not the screen presenter. Confidence is **high** for that specific
  handler.
- **UNKNOWN:** other KPPMainApp modules may still participate in Notebook
  lifecycle or content preparation; this finding does not exonerate the entire
  executable from every presentation role.

### E — another component reports display completion

- **FACT:** the reviewed candidate set is not exhaustive proof of all stock
  display components.
- **UNKNOWN:** an unobserved compositor, display service, callback library, or
  other process could own the setter. No evidence currently ranks a specific
  additional component.

### Current ownership conclusion

**FACT:** Powerd owns the observed set-to-one and zero-store paths. The
set-to-one store follows return from powerd's `goingToScreenSaver` send helper;
it is not a value reported by blanket, Awesome, or KPPMainApp.

**FACT:** Powerd later reads the same global in a state-machine path and selects
between calls into an internal routine containing a low-power frontlight
diagnostic. Powerd is therefore both the demonstrated owner and consumer.

**FACT:** KPP's specifically named `ApplicationModule::screenSaverHandler()`
is registered for both entry and exit events and performs engagement-metrics
bookkeeping. It is not a presentation-completion report to powerd.

**INFERENCE:** The field is more likely logical transition/frontlight state
than window visibility, framebuffer modification, update submission, or panel
refresh completion. Static evidence does not establish the formal meaning.

**FACT:** The target blanket launcher is a loader-driven X11 event-loop host;
its demonstrated code does not directly call a framebuffer, Cairo drawing,
Lab126 graphics, or E-Ink update interface (OBS-011 / FACT-023).

**FACT — later target-library evidence (OBS-014 / FACT-026):** Blanket resolves
the `screensaver` loader to `/usr/lib/blanket/screensaver.so`, which target
pathname metadata later resolves to `/usr/lib/blanket/screensaver.so.1.0`
(OBS-015 / FACT-027). The common library has X11 screensaver-window and
fbdev/image capabilities.

**FACT — target-plugin evidence (OBS-016 / FACT-028):** The copied plugin
consumes `goingToScreenSaver`, prepares/renders screensaver content, creates a
Cairo Xlib surface for `blanket_screensaver`, and brings that window up. It is
therefore the demonstrated content/window presenter.

**UNKNOWN:** Neither the plugin, blanket host, Pillow, Awesome, nor another
component has been attributed as the physical E-Ink update submitter. This new
presentation attribution does not change the earlier finding that powerd sets
`g_is_screensaver_drawn` internally rather than receiving a plugin completion
acknowledgement at that store.

## Presentation-completion interfaces

| Interface or term | Evidence | Interpretation |
| --- | --- | --- |
| `g_is_screensaver_drawn` | Target powerd log plus local setter/consumer disassembly | Powerd-owned internal state; physical-presentation semantics unsupported and still **UNKNOWN** |
| `isScreenSaverLayerWindowActive` | Generic read-only winmgr property; target awake query timed out | Possible layer-visibility observable; relationship to powerd **UNKNOWN** |
| `screensaver_showing` | Generic read-only advertising-blanket property | Possible ad-screensaver visibility observable; no-ads target applicability **UNKNOWN** |
| `activeSS`, `screenSaver`, `L:SS` | Generic Awesome window roles/layer | Window/layer vocabulary, not a completion callback |
| `screensaver.so.1.0` `goingToScreenSaver` callback | Target plugin render path reaches `blanket_image_window_bringup` | Demonstrated content/window presentation boundary; not a completion acknowledgement or panel-update proof |
| `kpp::app::ApplicationModule::screenSaverHandler()` | Target KPP dynamic symbol, registration call sites, and function body | Entry/exit lifecycle-metrics consumer; not a demonstrated presentation boundary |
| blanket `load` / `unload` `screensaver` | Generic interface plus KOReader behavior | State-changing module control, not an approved observation or proven completion boundary |
| `readyToSuspend` | Target-observed powerd event | Suspend-readiness event; not proof that a screen is drawn |
| draw/refresh completion | Present in third-party framebuffer implementations | Does not identify the stock target interface or owner |

No reviewed evidence exposes a target-proven presentation-completion interface.
The local analysis specifically shows that the powerd field is not an external
draw-completion acknowledgement and that KPP's named application-module handler
is metrics-oriented. The read-only layer/status getters remain possible runtime
observables, not controllable boundaries.

## Relationship to suspend

**FACT:** In OBS-006, the value 1 was logged after kernel resume as powerd moved
from `SUSPENDED` to `SCREEN SAVER`. The log did not record a value transition,
screensaver entry, `readyToSuspend`, or kernel suspend entry in the same retained
sequence.

**FACT:** Phase 2 separately observed `goingToScreenSaver`, repeated
`readyToSuspend`, and kernel `mem` entry, but did not observe this field during
that entry-side interval.

**FACT — static scope:** In the identified set-to-one path, powerd sends
`goingToScreenSaver`, stores 1, logs the value, and may subsequently call the
imported `device_can_suspend`. The store is not passed as an argument to that
call. A separate state-machine path reads the value for a branch tied to an
internal low-power frontlight routine.

**INFERENCE:** The static evidence weakens the hypothesis that the value is a
presentation-completion gate. Its proximity to `device_can_suspend` may reflect
state-transition sequencing rather than a causal prerequisite.

**UNKNOWN:** Whether another powerd path explicitly tests the value when
deciding to enter `readyToSuspend`, whether it survives suspend, and exactly
when its zero-store paths execute. Static order alone cannot establish runtime
causality.

## Minimum target-side static inspection design

The public/pinned search does not resolve the field. The smallest next step is
a staged, terminal-only string search over the four already-identified
executables. It does not execute or alter those binaries.

### Stage 0 — availability gate

```sh
command -v strings
```

- Safety classification: **READ_ONLY**.
- Purpose: determine whether a static string-extraction utility is already
  installed.
- Why read-only: shell command lookup reads command tables and executable-path
  metadata; it does not install or invoke `strings`.
- Availability risk: it may print nothing and fail. If so, stop; do not install
  a tool or substitute a broader binary dumper.

### Stage 1 — powerd field and local string context — completed

Run only if Stage 0 returns a path:

```sh
strings /usr/bin/powerd |
grep -E -B 12 -A 12 'g_is_screensaver_drawn|is_screensaver_drawn|screensaver_drawn|screen_saver_drawn|isScreenSaverDrawn|screenSaverDrawn|ScreenSaverDrawn|screensaver drawn|screen saver drawn|screensaver_showing|screenSaverShown|screensaver_shown|isScreenSaverLayerWindowActive|activeSS|screenSaver|readyToSuspend|drawComplete|draw_complete|draw complete|refreshComplete|refresh_complete|refresh complete'
```

- Safety classification: **READ_ONLY**.
- Purpose: determine whether the exact field and nearby callback/interface
  strings are embedded in powerd; limited context may expose an adjacent log
  format or callback name.
- Why read-only: `strings` reads bytes from one known executable and writes
  printable sequences to a pipe; `grep` filters that stream. Neither executes
  the binary or writes a file/property.
- Availability risk: the installed `strings` or `grep` may not support the
  assumed behavior/context flags; a stripped binary may retain only the log
  format; file-offset order is not call-graph evidence.
- Observer effect: one bounded sequential read may add short CPU and storage
  read activity while the device is awake. It does not observe or alter a sleep
  transaction.

**Observed result (OBS-007 / FACT-019):** The exact value diagnostic and a
separate reset-to-false diagnostic were present. The output also included
blanket configuration-key names, screensaver/readiness event names, and
low-temperature frontlight-management diagnostics. No set-to-true string or
explicit completion callback was identified. Context is printable-string file
order, not a call graph.

### Stage 2 — exact cross-candidate string comparison — completed

Run only if Stage 1 leaves the producer unresolved:

```sh
for f in /usr/sbin/blanket /app/bin/KPPMainAppV2 /usr/bin/awesome; do
    printf '\n=== %s ===\n' "$f"
    strings "$f" |
    grep -E 'g_is_screensaver_drawn|is_screensaver_drawn|screensaver_drawn|screen_saver_drawn|isScreenSaverDrawn|screenSaverDrawn|ScreenSaverDrawn|screensaver drawn|screen saver drawn|screensaver_showing|screenSaverShown|screensaver_shown|isScreenSaverLayerWindowActive|activeSS|screenSaver|readyToSuspend|drawComplete|draw_complete|draw complete|refreshComplete|refresh_complete|refresh complete'
done
```

- Safety classification: **READ_ONLY**.
- Purpose: look for an exact shared completion/property/callback name in only
  the three remaining candidate executables.
- Why read-only: the loop opens only the named executable files for string
  extraction and filters stdout; it executes none of them and writes nothing.
- Availability risk: absence of a string may mean stripping, indirection, a
  shared library, encoding, or an incorrect candidate—not absence of the
  behavior. KPPMainAppV2 may yield more matches than the native binaries.
- Privacy/licensing: static executables should not contain notebook data, but
  the output may contain proprietary implementation strings. Return only the
  short matching identifiers and immediately adjacent diagnostic lines needed
  for attribution; do not copy or commit a complete string dump.

**Observed result (OBS-008 / FACT-020):** KPPMainAppV2 contained the named
`kpp::app::ApplicationModule::screenSaverHandler()` symbol and
`readyToSuspend`, `screenSaver`, and `screenSaverTimeout` strings. No selected
match was emitted for blanket or Awesome. Thumbnail/TOC `draw complete`
messages are explicitly scoped to those content operations and are not treated
as screensaver-completion evidence. The generic clear-redraw message is also
unattributed.

### Stage 3 — focused symbol-table gate — stopped at availability

Stage 2 identifies KPPMainAppV2 but not a relationship. The smallest next read
is limited to symbol metadata for powerd's exact field and KPP's exact handler:

```sh
command -v readelf
```

```sh
readelf -Ws /usr/bin/powerd |
grep -E 'g_is_screensaver_drawn|screensaver_drawn'
```

```sh
readelf -Ws /app/bin/KPPMainAppV2 |
grep -E 'screenSaverHandler|readyToSuspend|screenSaverTimeout'
```

All three commands are **READ_ONLY**. `command -v` performs command lookup;
`readelf -Ws` reads the static and dynamic symbol tables of only the two named
executables; `grep` filters stdout. They do not execute or modify either file.
If `readelf` is absent, stop without installing or substituting a tool.

Availability and interpretation risk: either executable may be stripped, and
an absent symbol is inconclusive. A present symbol can establish object/function
type, binding, and address, but cannot establish callers or runtime ordering.
Return only matching rows; do not retain a full proprietary symbol dump.

**Observed result (OBS-009):** The target shell reported `readelf: not found`.
No symbol-table output was produced, so neither of the two filtered symbol
reads completed. Per the availability gate, no tool was installed and no `nm`,
`objdump`, disassembler, or other substitute is approved. This result says
nothing about whether either binary retains symbols.

No `nm`, `objdump`, service/config search, or disassembly command was approved
for the target at this gate. There were no **UNCERTAIN** or **STATE_CHANGING**
target commands in the approved set.

### Local follow-up — completed without device interaction

The owner subsequently copied the target `powerd`, `KPPMainAppV2`, and
`blanket` binaries to `docs/kindle_file/` and authorized local analysis.
OBS-010 and OBS-011 record their hashes. All three are stripped 32-bit
little-endian ARM EABI5 ELF executables with separate `.gnu_debuglink` names;
the referenced debug files were not supplied.

Local `objdump`, `strings`, `nm`, and checksum/file-metadata reads were
**READ_ONLY** with respect to the evidence artifacts. Neither ARM executable
was run. The analysis produced these narrow results:

- powerd's set-to-one path creates and sends `goingToScreenSaver` with an
  integer parameter, then stores 1 to global address `0x65814` and logs
  `g_is_screensaver_drawn = (1)`;
- two powerd paths store 0 to the same address, including the path with the
  explicit reset-to-false diagnostic;
- powerd later reads that address in a state-machine branch leading to an
  internal low-power/frontlight-management routine;
- KPP `registerScreenSaverListener()` maps `goingToScreenSaver` and
  `outOfScreenSaver` to the same `ApplicationModule::screenSaverHandler()`, and
  maps `readyToSuspend` separately to `deviceSuspendHandler(int)`; and
- the named KPP handler performs elapsed-time and engagement-metrics
  bookkeeping rather than a demonstrated draw-completion report; and
- the blanket executable parses named loaders, polls a blanket wakeup pipe and
  X11 connection, and delegates event dispatch to `libblanket.so.1.0`, without
  a direct imported presentation-completion report to powerd.

This local analysis does not approve any new target command. The copied stock
binaries are proprietary evidence artifacts with unknown licensing and must not
be staged as StayInk source.

## Decision gate

1. The exact field was **not found in pinned source**, but local analysis of the
   target binary resolved its stores and one consumer path.
2. Powerd is the demonstrated owner of both set-to-one and zero-store paths.
   The set-to-one store occurs after powerd's `goingToScreenSaver` send helper
   returns; it is not an external completion value supplied by KPP, blanket, or
   Awesome.
3. Powerd is also a demonstrated consumer. One state-machine branch reads the
   value and selects a path tied to low-power frontlight management.
4. KPP's specifically named `ApplicationModule::screenSaverHandler()` is a
   powerd entry/exit event metrics consumer, not a demonstrated presenter or
   draw-completion reporter.
5. The field cannot be assigned to window visibility, image preparation,
   framebuffer write, update submission, or panel-refresh completion. The
   evidence instead favors internal transition/frontlight bookkeeping.
6. Field lifetime, exact zero-store transitions, any independent suspend-gate
   use, and Scribe-generation portability remain **UNKNOWN**.
7. No new target command is approved. The local evidence artifacts should
   remain untracked and must not be treated as redistributable StayInk source.
8. `/usr/lib/blanket/screensaver.so.1.0` is the demonstrated content/window
   presentation boundary: its entry callback renders and brings up the
   screensaver X11 window. It is not yet a safely controllable boundary;
   physical framebuffer/panel ownership and recovery semantics remain unknown.
9. The project is not ready to design a reversible StayInk intervention. The
   next evidence must identify or correlate the actual visible replacement
   boundary without suppressing genuine suspend.
