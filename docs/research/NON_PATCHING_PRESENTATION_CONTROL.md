# Phase 3F — Non-patching presentation-control discovery

## Scope and evidence

This phase is offline, read-only static and documentation research. No Kindle
interaction occurred, no target artifact was executed or modified, and no
StayInk implementation or state-changing experiment was created. The target
scope is the UI-identified Kindle Scribe on firmware 5.19.5; its generation and
physical model number remain **UNKNOWN**.

The target artifacts are the same stripped, license-**UNKNOWN** files recorded
in OBS-014, OBS-016, and OBS-017:

- `libblanket.so.1.0`: SHA-256
  `30fb3dd09ee73ead89058d44a389a73b79c40d4192428e222c7fda9357459eef`;
- `screensaver.so.1.0`: SHA-256
  `864a9987ea0c556f7e837d7433c265f862845973444cc74aeba3674ed77acf4e`.

The proprietary artifacts and full disassembly are not research-document
content and must not be committed. OBS-018 records this focused inspection.
Generic Awesome documentation comes from SRC-003 at pinned commit
`cf4457c4808636dac5e06d2f2761cb4632efe66d`; KOReader behavior comes from
SRC-004 at pinned commit `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`.
Neither source proves behavior on Scribe firmware 5.19.5.

## Stock screensaver window identity

### Creation and lifetime

**FACT — target artifacts, high confidence (OBS-018 / FACT-030):** The
screensaver plugin's `init` path calls `blanket_image_get_window` with the
identifier literal `blanket_screensaver`, then creates a Cairo Xlib surface for
the returned XID. In the common library's relevant type-0 path,
`blanket_image_get_window`:

1. returns the previously stored XID if the common context already has one;
2. otherwise obtains a base name from
   `win_mgr_utils_new_screen_saver_name()`;
3. adds identifier `blanket_screensaver` with `win_mgr_utils_add_id`;
4. adds flash-on-show mode 3 and flash-on-hide mode 3;
5. adds the configured after-show damage timeout;
6. adds orientation value 1;
7. adds a custom property named `loc_module` with a configuration-derived
   value whose exact contents remain **UNKNOWN**;
8. serializes that metadata with `win_mgr_utils_to_string`;
9. creates the X11 window with `XCreateWindow`;
10. stores the serialized string as the window title with `XStoreName`;
11. installs `_NET_WM_PING` and `_NET_WM_PID`, flushes/synchronizes X11, and
    stores the XID in the common context.

Normal `goingToScreenSaver` and `outOfScreenSaver` paths map and unmap this
window; they do not destroy it. Plugin deinitialization destroys it.

**FACT — scoped negative finding:** No `XSetClassHint`, `WM_CLASS` literal, or
equivalent class-setting call was found in the reviewed creation and plugin
paths. This does not prove that `WM_CLASS` is absent at runtime or that another
library cannot set it.

**FACT — target runtime snapshot (OBS-022):** The stock-UI tree contained one
unmapped, full-screen candidate at XID `0x400001` with exact displayed title:

```text
L:SS_N:screenSaver_O:U_FH:F_module:screensaver_ID:blanket-screensaver_FS:F
```

It had empty displayed class parentheses, geometry 1860x2480+0+0, and
`MapState=IsUnMapped`. The internal identifier input's underscore is therefore
not preserved literally in the serialized ID: the runtime field is
`ID:blanket-screensaver`. The internal module-related metadata appears as
`module:screensaver`. Formal meanings of `FH:F` and `FS:F` remain **UNKNOWN**;
they are consistent with, but do not by themselves define, the statically
observed flash-on-hide/show builder calls.

**INFERENCE:** Within one uninterrupted plugin lifetime, normal sleep/wake
cycles are intended to reuse the same stored XID. Reuse across multiple target
cycles has not yet been observed directly.

### What is exact and what remains unknown

| Property | Evidence status | Result |
| --- | --- | --- |
| Logical identifier | **FACT** | Static input is `blanket_screensaver`; runtime serialized field is `ID:blanket-screensaver`. |
| X11 title / `WM_NAME` | **FACT** | The exact displayed target title in OBS-022 is `L:SS_N:screenSaver_O:U_FH:F_module:screensaver_ID:blanket-screensaver_FS:F`. |
| Layer / role | **FACT** | Runtime fields are `L:SS` and `N:screenSaver` in OBS-022. |
| `activeSS` | **FACT** as an unrelated library literal | It occurs in another `blanket_image_get_window` mode. The reviewed screensaver plugin uses the type-0 screen-saver-name path, so this literal does not establish its role. |
| `WM_CLASS` | **FACT** for displayed tree field, scoped | OBS-022 showed empty class parentheses for this window; no setter was found in the reviewed creation scope. A dedicated property value remains unavailable without `xprop`. |
| `_NET_WM_PID` | **FACT** | Set to the creating process PID as a 32-bit cardinal. |
| `_NET_WM_PING` | **FACT** | Installed as a supported window-manager protocol. |
| Lab126 X properties | **FACT** for title metadata; scoped negative for separate properties | Runtime title contains layer, role, orientation, `FH:F`, `module:screensaver`, ID, and `FS:F`. Static code adds the related builder inputs, including internal `loc_module`. No separate Lab126-specific `XChangeProperty` call was found in the reviewed creation path. |
| Initial map state | **FACT** by the creation/map split | `XCreateWindow` creates the window before the later bringup helper maps it. |

SRC-003 documents the general Kindle Awesome title format as an
underscore-separated metadata store. It documents `L:SS` as the screensaver
layer, above the application layer; names `activeSS` and `screenSaver` as known
roles without defining them; and documents a `HIDE` flag whose removal shows a
window. These are useful decoding leads, not proof of the exact title or
behavior on this target firmware.

## Reviewed winmgr, blanket, and Awesome controls

### LIPC inventory

**FACT — pinned documentation, generic applicability:** The focused
`com.lab126.winmgr` inventory contains:

- read-only `isScreenSaverLayerWindowActive`;
- read-only `getActiveAppTitle`;
- write-only, TODO-documented `visibleWindows` and `getAllWindows`;
- several display/debug controls that are unrelated or unsafe for this phase.

The target's one awake-state read of
`isScreenSaverLayerWindowActive` timed out during Phase 2A. Neither the property
name nor the generic inventory proves that it works on Scribe 5.19.5 or that it
identifies `blanket_screensaver` specifically.

**FACT — scoped negative finding:** No reviewed winmgr LIPC property provides a
documented, narrow, reversible command to hide, unmap, lower, or restore only
the stock screensaver window. TODO/write-only entries are not approved as
controls.

The reviewed blanket inventory exposes module `load`, module `unload`, and
write-only `uiQuery`. SRC-004's Kindle device code reads the `load` property
with `get_string_property` and interprets its result as the currently loaded
blanket-module names. This supports one narrow read-only discovery getter; it
does not make writing `load` or `unload` safe. Phase 3E established that
whole-module unload removes callbacks and destroys plugin state. No reviewed
evidence gives `uiQuery` window-visibility semantics. It is therefore not a
presentation-only control.

### Awesome title metadata

**FACT — SRC-003, generic applicability:** Kindle Awesome interprets the X11
window title as metadata, including layer, role, ID, and the `HIDE` flag.

**INFERENCE — target applicability unknown:** Replacing the persistent
screensaver window's title with an otherwise identical title containing
`HIDE`, before the stock map, is the strongest known non-patching way to ask
Awesome to suppress presentation while leaving the plugin loaded. Restoring
the exact original title would be the natural rollback. This is not yet an
approved experiment: target recognition of `HIDE`, focus behavior, Expose
behavior, and restoration have not been observed.

No reviewed stock Awesome command interface provides a narrower operation.
Changing the title or any X11 map/stacking state would be **STATE_CHANGING** and
is outside this phase.

## External X11-client feasibility

**FACT — X11 architecture, target access unknown:** A client that can connect
to the same X server and knows an XID can request window attributes/properties,
`XUnmapWindow`, `XMapRaised`, `XLowerWindow`, or title/property changes. These
requests do not require cooperation from the client that created the window.

**UNKNOWN — target:** Root identity alone does not prove X-server access. The
target display name, Unix-socket permissions, X authority/cookie policy,
security extensions, and access from a future root-launched process have not
been established.

**FACT — SRC-004, other Kindle environments only:** KOReader ships and uses a
GPLv2 `wmctrl` binary on Kindle to list and resize another client's title-bar
window. Its launcher comments report that hiding the title bar can trigger
`ligl` timeouts and that firmware 5.12.4 and later exhibited a UI softlock on
exit even though the `wmctrl` operation succeeded. This proves external X11
manipulation is possible in some Kindle environments and is also direct
evidence of risk. It does not establish Scribe 5.19.5 access, screensaver
behavior, or safety, and no third-party implementation is copied here.

**INFERENCE:** External unmap/hide is architecturally feasible if the target
connection gate succeeds. It is not yet operationally safe.

## Awesome reaction and state consistency

**FACT — target plugin:** The plugin issues one bringup during its normal
unpresented-to-presented entry branch and then records presentation state 1.
No reviewed normal-entry path continuously remaps the window or rewrites its
title. Normal exit unmaps when the stored state is nonzero and then resets the
state.

**INFERENCE:** If another client unmaps the window after bringup, the plugin
will remain internally “presented” until normal exit. Its later redundant
unmap is likely tolerated by X11, but that does not establish stock runtime or
wake safety.

**UNKNOWN:** Target Awesome may unmanage, remap, raise, or restack a
screensaver-layer client after an external unmap/lower/remap. The target
Awesome rules/configuration were not available in the reviewed artifacts, and
the pinned documentation does not specify these transitions.

**INFERENCE:** Lowering is less promising than a pre-map hide request. Awesome
has an explicit screensaver-above-application layer policy and may reassert
that stacking order. Changing the layer metadata itself could also alter
focus, chrome, damage, and wake behavior.

## Map, Expose, rendering, and flash ordering

**FACT — target artifacts, high confidence (OBS-018 / FACT-031):** The precise
static sequence is:

```text
blanket_image_window_bringup
  -> XSelectInput
  -> XMapRaised
  -> XFlush
  -> return; plugin stores presentation state = 1

asynchronous X11 Expose callback, event type 12 and final expose count
  -> validate Cairo surface/context
  -> cairo_paint
  -> if a deferred default/book-cover flag is set:
       delay
       -> win_mgr_utils_create_flash_trigger
       -> deferred render
       -> clear the flag
```

The Expose callback begins at plugin address `0x407c`. The base `cairo_paint`
occurs before either deferred flash-trigger branch. The deferred render occurs
after creation of its flash trigger.

**INFERENCE:** An observer that waits until the window is mapped before
unmapping it is inherently racy. The Expose callback may already have painted
or created a flash trigger before the external request is processed.

**UNKNOWN:** Whether an immediate external unmap prevents, races with, or
follows the visible E-Ink replacement; whether unmap damage causes Awesome or
the Notebook client to repaint; and whether such a repaint produces a panel
update. `blanket_image_window_teardown` itself only calls `XUnmapWindow` and
`XFlush`. X11 damage/exposure semantics do not establish physical E-Ink
behavior, and no deeper HWTCON attribution is needed at this decision point.

## Ranked non-patching candidates

| Rank | Candidate | Lifecycle and state | Reversibility | Principal risk | Confidence |
| --- | --- | --- | --- | --- | --- |
| 1 | Pre-stage the persistent window's existing Awesome title metadata with `HIDE`; restore the byte-exact original title | Preserves plugin callbacks, content/cache/LIPC work, stock map call, and plugin state 1 | Runtime title restore; reboot/plugin recreation is a secondary recovery path, not the planned rollback | Target may ignore `HIDE`; mapping a hidden client may still cause Expose/flash; focus or wake rules may change | medium-low |
| 2 | External `XUnmapWindow` immediately after stock map | Preserves entry lifecycle but leaves plugin state 1 while the X window is unmapped | Normal exit plus explicit remap/unmap recovery are conceptually available | Post-map Expose/flash race; Awesome may reassert; underlying page may not repaint | low |
| 3 | Lower the window or replace only its screensaver layer metadata | Preserves the window and plugin state | Original stacking/title can be restored | Awesome may restack `L:SS`; layer change may affect focus, chrome, damage, and wake | low |

No candidate requires a persistent startup change, rootfs modification, LIPC
property write, binary patch, or framebuffer access in principle. Every
candidate is nevertheless **STATE_CHANGING** and remains unapproved.

The title-metadata candidate is strongest because it can be established before
the stock map and therefore has a better chance of avoiding the after-map
race. It also preserves the plugin's own “presented” state, unlike a helper
no-op or post-map external unmap. Static evidence does not yet show that it
prevents the Expose/flash path or remains hidden through suspend.

## Minimum read-only target observation gate

The first target interaction should identify the one persistent window while
the device is awake, capture its displayed title and available X11
attributes/window-manager hints, and establish whether the installed read-only
X client can connect. It must not change a title, property, map state, layer,
service, tracing state, framebuffer, or powerd state.

### Stage 1 — availability gate

Run manually while the Kindle is awake:

```sh
command -v xwininfo
command -v xprop
```

| Command | Safety | Purpose and stop rule |
| --- | --- | --- |
| `command -v xwininfo` | **READ_ONLY** | Checks whether the standard query client exists; it does not execute it. |
| `command -v xprop` | **READ_ONLY** | Checks whether the standard property query client exists; it does not execute it. |

**FACT — target availability report (OBS-019):** `xwininfo` is available on the
observed Scribe and `xprop` is not. The exact `xwininfo` path/version and its
supported option inventory were not included in the report.

Absence of `xprop` does not justify package installation, copying a client, or
substituting an unreviewed binary. It narrows the first observation to the
standard read-only information `xwininfo` can expose. Arbitrary properties,
`WM_CLASS`, and `_NET_WM_PID` remain **UNKNOWN** after this reduced path.

### Stage 2 — exact-window discovery and query

First verify that `xwininfo` can query the current X server without hiding an
error behind `grep`, and independently query the current blanket module list:

```sh
xwininfo -root -stats 2>&1
lipc-get-prop com.lab126.blanket load
```

| Command | Safety | What it can establish |
| --- | --- | --- |
| `xwininfo -root -stats 2>&1` | **READ_ONLY** | Proves whether this client can connect and query only the root window. Any error is preserved in the output. |
| `lipc-get-prop com.lab126.blanket load` | **READ_ONLY** | Uses only the LIPC getter. SRC-004 treats its result as the current loaded-module list. It does not request module loading. |

If the root query fails, stop. If the blanket getter fails or its returned list
does not contain `screensaver`, stop: the current environment cannot establish
the stock screensaver window's X11 identity.

**FACT — target observation (OBS-021):** In the stock-UI foreground condition,
the root query succeeded and identified root XID `0x50` as a viewable
1860-by-2480, depth-8 `StaticGray` `InputOutput` window. The blanket getter
returned `screensaver langpicker blankwindow usb`. Thus the OBS-020 empty exact
filter was not caused by X connection failure or by `screensaver` being absent
from blanket's reported loaded-module list.

Only when the root query succeeds and `screensaver` is reported loaded, run:

```sh
xwininfo -root -tree 2>&1 | grep -F 'blanket_screensaver'
```

This is **READ_ONLY**: `xwininfo` queries the existing root-window tree and
`grep` restricts returned output to the exact known identifier. It is a
one-time bounded enumeration because the XID and exact serialized title are
currently unknown. If it returns zero or more than one candidate, stop and
copy the result for review; do not infer that the target window does not exist.

OBS-020 returned zero exact-string matches despite the successful connection
and loaded-module facts later established by OBS-021. The next and only broader
query approved at this gate is:

```sh
xwininfo -root -tree 2>&1 | grep -Ei 'L:SS|activeSS|screen.?saver|blanket|1860x2480'
```

This is **READ_ONLY** and bounded to the known screensaver metadata terms plus
the target's exact full-screen geometry. It may include unrelated full-screen
windows; copy the complete filtered output without selecting a candidate. If
it returns no result, stop rather than printing the unfiltered window tree.

**FACT — OBS-022:** This broader query returned exactly one `L:SS` /
`N:screenSaver` window, XID `0x400001`, with the exact title and unmapped
full-screen attributes recorded above. It also returned an unrelated unmapped
KPP application window and the viewable `blankwindow`; their presence does not
establish which stock application was visually foregrounded or who owns a
panel refresh.

For exactly one result, manually substitute its XID below; do not use automatic
PID/XID discovery logic:

```sh
xwininfo -id 0xXID -stats -wm
lipc-get-prop com.lab126.winmgr isScreenSaverLayerWindowActive
```

| Command | Safety | What it can establish |
| --- | --- | --- |
| `xwininfo -id 0xXID -stats -wm` | **READ_ONLY** | The selected window's displayed title, geometry, map state, and standard window-manager hints. If this target build rejects `-wm`, stop and copy the error; do not try undocumented options. |
| `lipc-get-prop com.lab126.winmgr isScreenSaverLayerWindowActive` | **READ_ONLY** | A single retry of the already-reviewed getter, correlated with the awake X map state. Timeout or failure is evidence only; do not retry repeatedly or substitute a setter. |

**FACT — target observation (OBS-023):** The direct XID query reconfirmed the
exact title, full-screen geometry, depth-8 `StaticGray` visual,
`IsUnMapped`, and `Override Redirect State: no`. It reported no conventional
window-manager hints, desktop 0, and process ID 4524. That PID matches the
previously observed `com.lab126.blanket` owner mapped to `/usr/sbin/blanket` in
FACT-013. The simultaneous winmgr getter returned `0`.

**INFERENCE:** Getter value `0` is consistent with the directly observed
unmapped screensaver-layer window. A single awake correlation does not prove
that the property formally tracks this XID, every `L:SS` client, or physical
presentation.

The observation should record the exact command output, firmware, awake UI,
foreground application, wall-clock time, and whether KOReader is running. It
does not require a sleep cycle. It cannot establish mapped-state timing,
physical panel behavior, `HIDE` semantics, arbitrary X properties, the owning
PID property, or a class hint.

If `xwininfo` has unfamiliar provenance or options on the target, classify its
execution **UNCERTAIN** and stop until that exact binary is reviewed. Do not use
`wmctrl`, broad LIPC/DBus enumeration, polling, an X event injector, or a
detached observer as a convenience substitute.

## Experiment gate

A first reversible state-changing experiment is justified only if the read-only
observation shows all of the following:

1. exactly one candidate window has runtime identifier
   `ID:blanket-screensaver`;
2. `xwininfo` displays the full original ASCII title without truncation or
   ambiguous escaping, allowing it to be captured and unambiguously parsed;
3. the title includes stock screensaver-layer/role metadata consistent with
   SRC-003;
4. the window is unmapped while awake and its XID remains addressable;
5. a root-launched read-only X client can query it without changing behavior;
6. there is an exact, manually auditable textual restore value; if the title is
   non-ASCII or `xwininfo` escapes it ambiguously, this gate fails;
7. no evidence indicates that querying the window perturbs Awesome, Notebook,
   powerd, or wake behavior.

That result would justify designing—**not yet executing**—one bounded trial of
the pre-map `HIDE` metadata candidate with one changed variable, a timed restore,
normal physical wake, verified Notebook restoration, and pre/post
`suspend_stats` plus kernel PM evidence. Failure of any gate keeps Phase 3F in
research and does not justify falling back to post-map unmap, layer changes,
module unload, binary patching, or framebuffer intervention.

**Gate result — OBS-022/OBS-023:** Items 1 through 6 are directly satisfied for
this runtime snapshot. The queries produced no reported UI or power-management
change, satisfying item 7 only at observation confidence rather than proving
absence of every observer effect. Phase 3F now provides enough evidence to
begin a separate design review for the first reversible `HIDE`-metadata
experiment. It does not authorize execution.

## Phase 3F conclusion

**FACT:** The exact stock object in OBS-022 is XID `0x400001`, an unmapped
1860-by-2480 X11 window with layer `SS`, role `screenSaver`, and identifier
`blanket-screensaver`. Its serialized Awesome title carries the presentation
policy. No verified stock LIPC visibility setter was found.

**INFERENCE:** A reversible `HIDE` title flag applied before map is the
strongest current non-patching control candidate because it preserves the most
plugin lifecycle and avoids the known post-map race.

**FACT — target observation (OBS-020):** The first exact-string filtered tree
query returned no output while the stock Amazon UI, not KOReader, was in the
foreground. Because the pipeline filtered standard error and the current
blanket module list was not captured, this is not evidence that the window is
absent. Stock UI foreground does not independently establish that every
KOReader-related background publisher/process was absent.

**FACT — target observation (OBS-021):** The same stock-UI condition had a
working X root query and blanket reported `screensaver` loaded.

**FACT — target observation (OBS-022):** The broader filter resolved the exact
runtime title and showed that underscore-to-hyphen serialization explained the
original literal mismatch.

**FACT — target observation (OBS-023):** Direct XID metadata tied the window's
reported PID to blanket and correlated its unmapped state with winmgr getter
value `0`.

**UNKNOWN:** XID reuse across cycles, formal `FH:F`/`FS:F` semantics, target
recognition of an added `HIDE`, Awesome's reaction, whether a hidden mapped
window receives Expose and flash work, physical panel preservation, genuine
suspend equivalence, and normal wake restoration. The project is ready to
design, but not execute, a state-changing trial.
