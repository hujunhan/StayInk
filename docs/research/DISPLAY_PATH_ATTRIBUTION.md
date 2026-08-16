# Phase 3A — Display-path attribution

## Status and scope

Phase 3A is a focused attribution study. The device owner ran only the two
reviewed read-only existing-log commands recorded below. No display, power,
service, tracing, or framebuffer state was changed by the observation.

The concrete question is: on the observed Kindle Scribe running firmware
5.19.5, which stock component or components cause the visible Notebook-to-
screensaver replacement after `goingToScreenSaver` and before kernel `mem`
suspend?

The device generation and physical model number remain **UNKNOWN**. The target
is jailbroken with Véra and KPM, and KOReader has been present; it is not a
proven pure-stock boot. Phase 2 already established the relevant powerd-to-
kernel ordering in both KOReader and reported stock-Notebook foreground
conditions. Phase 3A does not reopen kernel suspend attribution.

The observation that would materially answer this question is a target-scoped,
timestamped record showing which candidate reports creating, selecting,
showing, hiding, or activating the screensaver window/layer at the physical
replacement. Component and publisher names alone are insufficient.

## Evidence basis and licensing

- **OBS-001 / FACT-013:** direct PID-to-executable observations on the target,
  recorded in `SOURCES.md` and `FACTS.md`.
- **OBS-002 and OBS-005:** direct target power-event and kernel observations,
  recorded in `POWER_TRACE.md`.
- **OBS-006 / FACT-018:** direct target read of matching records already present
  in `/var/log/messages`; raw output is not retained because it included
  notebook/content identifiers.
- **SRC-003:** KindleModding documentation at commit
  `cf4457c4808636dac5e06d2f2761cb4632efe66d`, CC BY-NC 4.0. Relevant locations
  are `content/kindle-apps-and-services/com.lab126.blanket.md:8-16`,
  `content/kindle-apps-and-services/com.lab126.winmgr.md:8-41`,
  `content/kindle-apps-and-services/com.lab126.pillow.md:6-24`,
  `content/kindle-dev/awesome-window-manager/__index.md:16-74`, and
  `static/kindle-hacking/upstart-diagram.html:145-175,245-260,330-340`.
  Its LIPC descriptions are mostly `TODO`, and its service inventory and
  Upstart graph are not Scribe-5.19.5 evidence.
- **SRC-004:** KOReader at commit
  `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`, AGPL-3.0. Relevant behavioral
  references are `frontend/device/kindle/device.lua:329-383,750-845,2048-2054`
  and `platform/kindle/koreader.sh:232-280,361-390`.
  These establish KOReader behavior and community interpretation, not stock
  implementation or Scribe applicability. No source is copied into StayInk.

## Component attribution

### `blanket`

**FACT — target, high confidence:** On this Scribe/5.19.5 observation,
`com.lab126.blanket` was owned by `/usr/sbin/blanket`, and its command line was
`blanket -t screensaver langpicker blankwindow`. The process was root-owned and
had parent PID 3952. This proves only the executable identity and literal
arguments at that point in time (OBS-001 / FACT-013).

**FACT — target binary, high confidence (OBS-011 / FACT-023):** Local static
analysis of the copied `/usr/sbin/blanket` resolves `-t loader` as the name of
one or more loaders to initialize with blanket; its help permits a quoted,
space-separated value for multiple loaders. The executable passes that option
to imported `blanket_event_parse_loader`. The immediately following
`screensaver` token is therefore a loader argument. The earlier command-line
rendering replaced NUL separators with spaces, so it did not preserve whether
`langpicker` and `blankwindow` were part of the same quoted loader argument or
separate trailing arguments; their exact argv grouping remains **UNKNOWN**.

**FACT — target binary, high confidence (OBS-011 / FACT-023):** The executable
is a small X11 event-loop launcher directly dependent on `libblanket.so.1.0`.
It calls imported blanket initialization, loader parsing, event queue, dispatch,
and teardown functions; polls a blanket wakeup pipe and the X11 connection; and
queues pending X events for blanket dispatch. Although its dependency table
also names Lab126 graphics/window utilities, Cairo, rendering, LIPC, and other
libraries, the launcher contains no direct imported framebuffer, `ioctl`,
E-Ink-update, Cairo-drawing, or Lab126-graphics call.

**FACT — generic community inventory, medium confidence:** SRC-003 lists
`load`, `unload`, and `uiQuery` properties for `com.lab126.blanket`; the sampled
`load` value contains tokens including `langpicker`, `blankwindow`, and an
advertising-screensaver token. The descriptions are `TODO`, so the table does
not define their behavior or establish that the snapshot applies to the target.

**FACT — third-party behavior, high confidence within KOReader:** SRC-004 reads
the blanket `load` property as a set of loaded modules, unloads `splash` and
`screensaver` when KOReader's optional no-framework path is used, and reloads
`screensaver` on exit with a comment that this restores missing stock Amazon UI
screensavers (`device.lua:329-353,367-383,2048-2054`).

**INFERENCE — target applicability medium-high confidence:** Loader
initialization plus the X11 event loop makes blanket a presentation-path module
host and event coordinator. The exact behavior of the `screensaver` and
`blankwindow` loaders and whether initialization succeeded in a specific trial
remain **UNKNOWN**.

**INFERENCE — StayInk relevance medium confidence:** Blanket is a stronger
candidate presentation coordinator or module host than it was after Phase 2A.
Removing its `screensaver` module is associated in KOReader with missing stock
screensavers. That association does not identify which process constructs the
image, creates a window, writes pixels, or requests an update.

**UNKNOWN:** No blanket launch command or target-applicable service definition
was found in the reviewed pinned-source scope. The generic Upstart graph exposes
a `blanket_loaded_langpicker` event but not a blanket launch definition. PID
3952's identity is also unobserved.

**FACT — later target-library evidence (OBS-014 / FACT-026):** The supplied
`libblanket.so.1.0` resolves `screensaver` to the candidate shared object
`/usr/lib/blanket/screensaver.so`, whose target pathname was later observed as
`/usr/lib/blanket/screensaver.so.1.0` (OBS-015 / FACT-027) and whose ABI
includes LIPC, X11, and Kiwi callbacks. The common library provides
screensaver-window naming, X11 creation/map/unmap, Cairo, fbdev, and `ioctl`
helpers.

**FACT — target-plugin evidence (OBS-016 / FACT-028):** The plugin consumes
`com.lab126.powerd` `goingToScreenSaver`, `outOfScreenSaver`,
`exitingScreenSaver`, `userShutdown`, and `outOfShutdown` events. It obtains
the `blanket_screensaver` X11 window, creates a Cairo Xlib surface/context,
prepares/renders screensaver content, brings the window up on entry, and tears
it down on exit. This establishes content and X11 presentation ownership.

**UNKNOWN:** Which downstream component submits the E-Ink panel refresh. No
direct blanket-to-KPP or blanket-to-pillow call path was found. The plugin has
no direct framebuffer/HWTCON/MXCFB or E-Ink update import; its X11/window and
image calls do not by themselves identify the physical update submitter.

### `KPPMainApp` and `ScreenSaverListener`

**FACT — target, high confidence:** On this Scribe/5.19.5 observation,
`com.lab126.KPPMainApp.ScreenSaverListener` was owned by
`/app/bin/KPPMainAppV2` (process name `KPPMainApp`, command line
`/app/bin/KPPMainApp`) under UID 9000 (OBS-001 / FACT-013).

**FACT — scoped negative result, medium confidence:** No implementation of the
`ScreenSaverListener` publisher, its subscriptions, or its message handlers was
found in the reviewed pinned-source scope. SRC-003's generic Upstart graph only
shows `kppmainapp` following a `framework_ready` condition; this is neither a
target launch definition nor screensaver attribution.

**INFERENCE — low confidence:** The publisher name is consistent with
KPPMainApp observing screensaver lifecycle state. It may select content,
prepare a window, coordinate presentation, or merely update application state.
The word `Listener` does not select among those roles.

**UNKNOWN:** Its entry-side events/messages, whether it creates or selects
screensaver content, whether it delegates presentation, and its relationships
to blanket, pillow, and winmgr are not established. Publisher ownership does
not establish rendering ownership, event ordering, or panel-refresh ownership.

**FACT — target, high confidence:** OBS-006 later showed KPPMainApp receiving an
`onOutOfScreenSaverEvent`, notifying an observer on screensaver stop, and
running `NotebooksControllerImpl::OnScreenSaverStop` after powerd sent
`outOfScreenSaver`. Other KPPMainApp clients logged that the screensaver was
turning off and that `inScreenSaver` became 0. This directly establishes a
wake-side KPPMainApp lifecycle response on the target, but not that the named
`ScreenSaverListener` publisher was the delivery mechanism.

**INFERENCE — medium confidence:** KPPMainApp is now better supported as a
screensaver lifecycle/state consumer, at least on exit. This does not increase
confidence that it prepares or presents the sleep image, because the observed
handlers ran on the wake/stop side and reported no content or window action.

**FACT — target binary, high confidence (OBS-010 / FACT-022):** Local static
analysis shows that `ApplicationModule::registerScreenSaverListener()` maps
both powerd `goingToScreenSaver` and `outOfScreenSaver` to the same
`ApplicationModule::screenSaverHandler()`. That handler performs elapsed-time
and engagement-metrics bookkeeping. It is not a demonstrated content selector,
window presenter, or draw-completion reporter. `readyToSuspend` is registered
separately to `ApplicationModule::deviceSuspendHandler(int)`.

**UNKNOWN:** This resolves the specifically named application-module handler,
not every screensaver-related class in the large KPPMainApp binary. Another KPP
module could still participate in Notebook lifecycle or content preparation.

### `winmgr` / Awesome

**FACT — target, high confidence:** On this Scribe/5.19.5 observation,
`com.lab126.winmgr` was owned by `/usr/bin/awesome` (OBS-001 / FACT-013).

**FACT — generic community documentation, medium confidence:** SRC-003
documents a Kindle-specific Awesome window-title convention in which `L:SS`
selects a `SCREENSAVER` layer above the `APP` layer. It also lists window roles
named `activeSS` and `screenSaver`, without descriptions, and a `HIDE` flag that
controls window visibility (`awesome-window-manager/__index.md:16-74,124-132`).

**FACT — generic interface inventory, medium confidence:** SRC-003 lists a
read-only `com.lab126.winmgr.isScreenSaverLayerWindowActive` property but leaves
its semantics as `TODO` (`com.lab126.winmgr.md:8-41`). An awake-state target
query timed out during Phase 2A, so target usability is not established.

**INFERENCE — medium confidence:** Awesome likely participates in the visible
transition by ordering or exposing an already-created screensaver-layer window
above the Notebook application window. This is the clearest evidence for layer
and visibility management among the candidates.

**UNKNOWN:** The reviewed evidence does not show that Awesome selects the
screensaver content, writes framebuffer memory, or submits the E-Ink refresh.
Window/layer ownership is not panel-refresh ownership.

### `pillow`

**FACT — target, high confidence:** On this Scribe/5.19.5 observation,
`com.lab126.pillow` was owned by `/app/KPPPillow-2.0/bin/pillowd`
(OBS-001 / FACT-013).

**FACT — generic and third-party evidence, medium confidence:** SRC-003
describes Pillow as commonly displaying alerts and dialogs, lists chrome,
dialog, application-window, activity-indicator, and `pillowAlert` interfaces,
and places the `pillowAlert` role on the dialog layer. SRC-004 manipulates Pillow
to hide or restore chrome while KOReader runs (`koreader.sh:232-280,361-390`).

**INFERENCE — medium confidence:** Pillow is more strongly supported as a
chrome/dialog participant than as the primary stock screensaver presenter.

**UNKNOWN:** No reviewed evidence connects Pillow to the Scribe 5.19.5
screensaver replacement. It is therefore not included as an active element in
the candidate pipelines below.

### `blankwindow`

**FACT — target, high confidence:** `blankwindow` appeared literally in the
observed blanket command line.

**FACT — generic community inventory, medium confidence:** A matching token is
present in SRC-003's sampled blanket `load` value.

**UNKNOWN:** No definition or behavior for `blankwindow` was found in the
reviewed scope. Its name does not prove that it creates a window, blanks the
display, or participates in the observed replacement.

## Candidate display pipelines

These pipelines stop at an unknown display submitter. `hwtcon_v2` is the
observed framebuffer driver, but no evidence attributes a stock update request
to any candidate.

### Candidate 1 — blanket-hosted presentation, then Awesome layering

```text
powerd screensaver transition
  -> blanket screensaver module or blanket-coordinated presenter
  -> screensaver window/content
  -> Awesome exposes the SS-layer window above Notebook
  -> UNKNOWN display submitter
  -> hwtcon_v2 / panel update
```

- Supporting facts: blanket's target loader arguments; the target launcher's
  loader parser and X11 event loop; generic blanket module inventory; KOReader's
  unload/reload behavior and missing-stock-screensaver comment; Awesome's
  documented SS layer above the app layer.
- Unsupported assumptions: blanket receives the relevant powerd event;
  blanket creates/selects/shows the window; blanket communicates directly with
  Awesome; the layer change causes the update request.
- Confidence: **medium-high** that blanket is an X11 presentation-path module
  host and **medium** that Awesome also participates; **low** for the exact
  arrows and component that produces pixels.
- Distinguishing observation: a timestamped blanket action naming a screensaver
  module/window before a winmgr/Awesome SS-layer activation and before the
  physical replacement would favor this pipeline.

### Candidate 2 — KPP prepares content, Awesome presents it

```text
powerd screensaver transition
  -> KPPMainApp ScreenSaverListener
  -> KPP-selected or KPP-created screensaver window/content
  -> Awesome exposes the SS-layer window above Notebook
  -> UNKNOWN display submitter
  -> hwtcon_v2 / panel update
```

- Supporting facts: target ownership of the named KPP ScreenSaverListener
  publisher; OBS-006's target evidence that KPPMainApp handles the wake-side
  screensaver-stop lifecycle; OBS-010's proof that its application module also
  consumes entry events; Awesome's documented SS-layer behavior.
- Unsupported assumptions: a KPP module other than the metrics handler selects
  or creates content; KPP hands a window to Awesome; blanket is only a module
  host or background participant.
- Confidence: **low** for entry-side presentation. The specifically named
  `ApplicationModule::screenSaverHandler()` is now attributed to metrics, and
  no other KPP content/window action has been connected to the replacement.
- Distinguishing observation: a timestamped KPP ScreenSaverListener action
  naming content/window creation or show state before any blanket/winmgr action
  and the physical replacement would favor this pipeline.

A hybrid KPP-to-blanket-to-Awesome chain is possible, but the reviewed evidence
does not establish any of those inter-component arrows. It is not promoted to a
third pipeline merely because it can be imagined.

## Minimum next read-only target experiment

The next experiment should first reuse the already-completed Baseline B
transaction rather than trigger another sleep cycle. Search only the existing
system log for records emitted by the named candidate components or explicit
screensaver/window terms:

```sh
ls -l /var/log/messages
grep -Ei 'blanket|KPPMainApp|ScreenSaverListener|winmgr|awesome|pillow|blankwindow|screen.?saver' /var/log/messages
```

Both commands are **READ_ONLY**.

- `ls -l` reads directory-entry and file metadata to establish whether the log
  exists and its size/time; it does not open the log content or alter state.
- `grep` opens the existing log for reading and emits only matching lines to
  the terminal. It does not modify the log, candidates, IPC state, display, or
  power management.
- Availability risk: `/var/log/messages` may have rotated, may no longer retain
  the Baseline B interval, or may not contain candidate-tagged messages.
- Privacy risk: matching log lines may include document titles, application
  identifiers, account/device identifiers, or network information. Preserve
  timestamps and process/component tags, but redact unrelated personal values
  before returning the output.
- Observer effect: ordinary file reads and terminal/SSH activity occur after
  the completed transaction. They cannot observe live state and should not
  trigger another sleep cycle. The SSH session may keep networking active, but
  it is not part of the measured transaction.

Preserve the raw timestamp format. Correlate only records that can be placed in
the accepted Baseline B attempt-2 interval from existing evidence; do not infer
clock basis or timezone conversion from an unlabelled line.

Success criteria:

- Blanket-tagged create/load/show/window activity followed by an Awesome or
  winmgr SS-layer action favors Candidate 1.
- KPP/ScreenSaverListener content or window activity preceding the layer action
  favors Candidate 2.
- An Awesome/winmgr SS-layer record without a content owner strengthens its
  layer-coordinator role but leaves the producer unresolved.
- Pillow-only chrome/dialog activity does not establish screensaver ownership.
- No matching or useful lines is an inconclusive, scope-limited negative result.
  It must not be restated as absence of the relationship.

Do not substitute `showlog`, `logread`, broad `dbus-monitor`, broad
`lipc-probe`, framebuffer access, tracing, or a new sleep cycle at this gate.
If the two reads are inconclusive, stop for human review before designing a
more intrusive or live observer.

### Result of the existing-log read

**FACT — target, high confidence (OBS-006 / FACT-018):** The supplied matching
output covers a later wake-side sequence around local-wall-clock-like
`260814:231011`, not the accepted Baseline B entry/replacement interval. Powerd
logged `SUSPENDED -> SCREEN SAVER`, the literal field
`g_is_screensaver_drawn = (1)`, then `SCREEN SAVER -> ACTIVE` and
`outOfScreenSaver`. KPPMainApp subsequently logged its wake-side lifecycle
handlers described above.

**FACT — scoped negative result:** The supplied matches contain no
`goingToScreenSaver` line and no blanket, winmgr/Awesome, pillow, or blankwindow
action. They also contain no KPP action naming sleep-image selection, window
creation, show state, or layer activation. This statement applies only to the
retained log content and the approved search expression.

**INFERENCE:** The log may have rotated or ceased to retain the relevant
Baseline B interval, but the exact retention history is **UNKNOWN**. The file's
size and modification time do not by themselves establish rotation.

**FACT — target binary (OBS-010 / FACT-021):** Powerd itself sets and clears
`g_is_screensaver_drawn`; the set-to-one store follows its
`goingToScreenSaver` send helper. The field is not a completion value reported
by blanket, KPPMainApp, or Awesome at that setter.

**UNKNOWN:** The field's formal meaning and relationship to presentation. It
does not identify the renderer, window owner, framebuffer writer, or
panel-refresh submitter.

**Decision:** The read is useful but inconclusive for the entry-side display
pipeline. It confirms KPPMainApp as a wake-side lifecycle consumer; it does not
favor Candidate 1 or Candidate 2 as the sleep-screen presenter. Per the defined
gate, no broader log collector or live observer is approved here.

## Phase 3A decision state

1. **Strongest current candidate:** the blanket-hosted
   `/usr/lib/blanket/screensaver.so.1.0` module is the demonstrated stock
   screensaver content renderer and X11 window presenter; Awesome likely
   manages the visible SS layer downstream.
2. **Evidence:** the plugin's callback table consumes `goingToScreenSaver`;
   its entry path prepares/renders default or book-cover screensaver content
   and calls `blanket_image_window_bringup` for `blanket_screensaver`; its exit
   path tears that window down.
3. **Still UNKNOWN:** the physical framebuffer/panel-update submitter, exact
   Awesome layer rule used at runtime, `blankwindow` behavior, any other KPP
   presentation role, and safe runtime suppression/recovery semantics.
4. **Blanket's likely role:** the executable is the demonstrated loader host
   and event coordinator; its `screensaver.so.1.0` loader is a demonstrated
   content renderer and X11 presenter. Neither is yet a proven physical-panel
   refresh owner.
5. **KPPMainApp's likely role:** target-active lifecycle/state consumption is
   established on both entry and exit for the named application-module handler,
   whose body is metrics-oriented. Other KPP content-selection, presentation,
   and delegation roles remain unestablished.
6. **winmgr/Awesome's likely role:** window visibility and screensaver-layer
   ordering; not proven to produce content or submit the E-Ink refresh.

There is now a strong static candidate presentation boundary: the plugin's
`goingToScreenSaver` render-to-window-bring-up path. There is not yet enough
evidence to design a reversible StayInk intervention. That gate still requires
verified runtime control semantics and a rollback path. A future reversible
experiment would also need to show that changing only that boundary preserves
successful kernel `mem` suspend, normal wake and UI restoration, and all wake
paths in scope. X11 presentation ownership alone does not establish safe
control or physical panel-refresh ownership.
