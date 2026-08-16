# Phase 3E — Presentation-boundary semantics

## Scope and evidence

This is offline, read-only static research on owner-supplied copies of the
target `/usr/lib/blanket/screensaver.so.1.0` and
`/usr/lib/libblanket.so.1.0`. Neither artifact was executed or modified, and
no Kindle interaction occurred. The target is the UI-identified Kindle Scribe
on firmware 5.19.5; its generation and physical model number remain
**UNKNOWN**.

The artifacts are the same stripped, license-**UNKNOWN** files recorded in
OBS-014 and OBS-016:

- `libblanket.so.1.0`: SHA-256
  `30fb3dd09ee73ead89058d44a389a73b79c40d4192428e222c7fda9357459eef`;
- `screensaver.so.1.0`: SHA-256
  `864a9987ea0c556f7e837d7433c265f862845973444cc74aeba3674ed77acf4e`.

OBS-017 records the focused Phase 3E inspection. The proprietary artifacts
and full disassembly are not research-document content and must not be
committed. Static control flow establishes possible and directly reached
operations, not that every conditional branch ran in a particular sleep
cycle or that suppressing one operation is runtime-safe.

## Exact entry callback

**FACT — target plugin, high confidence (OBS-017 / FACT-029):** The
`goingToScreenSaver` entries in the plugin callback table point to Thumb
callback address `0x3fa5`, whose body begins at `0x3fa4`. The embedded function
inventory identifies this entry path as the module's screensaver-map path.
The body follows this order:

```text
goingToScreenSaver callback (0x3fa4)
  -> emit screensaver navigation metric for goingToScreenSaver / NON-KSO
  -> inspect the incoming event parameter type
  -> replace the final callback argument's owned message string
       type 6: localized screenoff.asr.message
       otherwise: localized screensaver.asr.message
  -> select presentation mode
       type 6: small wrapper at 0x3f24 -> generic map state machine, mode 2
       otherwise: generic map state machine at 0x3d94, mode 1
  -> generic map state machine
       validate Cairo surface status
       if module presentation state == 0:
         -> prerender/content preparation at 0x3680
         -> on success, blanket_image_window_bringup(..., window)
         -> set module presentation state to 1
       if module presentation state == 1:
         -> if mode changed, prerender/content preparation at 0x3680
         -> cairo_paint on the already-presented Xlib surface
       otherwise:
         -> return without presentation work
  -> return the state-machine result

subsequent X11 Expose callback, when the window is mapped
  -> cairo_paint
  -> inspect deferred book-cover/default-screensaver paint flags
  -> create a win-manager flash trigger on the deferred-render branches
  -> perform the pending render and clear its flag
```

The call to `blanket_image_window_bringup` is at `0x3ea6`. Its return value is
not tested; the caller stores presentation state `1` immediately afterward.
This is important for any future suppression design: making only the helper a
no-op would not keep the plugin's own state synchronized with the X11 window.

### Action classification

| Order | Action | Class | Static interpretation |
| --- | --- | --- | --- |
| 1 | navigation metric emission | A — lifecycle/bookkeeping | Records entry; not presentation |
| 2 | inspect event parameter type | A/H — lifecycle/other | Chooses normal screensaver versus blank/screen-off mode |
| 3 | free and replace the final callback argument string | A — lifecycle/bookkeeping | Updates the event's localized ASR message data |
| 4 | clear/free prior module content and record mode | A/C — bookkeeping/file-cache state | Maintains per-module render state |
| 5 | read household/profile and book information on applicable branches | B/D — content selection and LIPC | Selects default versus book-cover content |
| 6 | prepare Cairo content/surfaces or set deferred paint flags | E/F — rendering/window preparation | Some branches render before map; others defer actual rendering to Expose |
| 7 | read/select/update default-screensaver rotation state on applicable branches | B/C — content selection and file/cache state | May touch `/var/local/blanket/screensaver/last_ss` before presentation |
| 8 | `blanket_image_window_bringup` | G — visible window presentation | Maps/raises the existing X11 window and flushes the request |
| 9 | set internal presentation state to 1 | A — lifecycle/bookkeeping | Occurs even though bringup's return is ignored |
| 10 | mapped-window Expose/repaint work | E/G — rendering/presentation | May perform deferred render and request a win-manager flash trigger |

**FACT:** Mode 2 fills the module Cairo context white before bringup. The
normal-content mode includes household/profile and book-information selection,
default/book-cover branches, Cairo preparation, and conditional deferred-paint
flags. The embedded diagnostics explicitly say that book-cover or default
screensaver rendering may be skipped during prerender and performed during
repaint.

**INFERENCE:** Mode 2 is the plugin's blank/screen-off presentation path, based
on the `screenoff.asr.message` branch, its white fill, and the embedded
`module_screensaver_map_blank` name. Its formal event-payload meaning remains
**UNKNOWN**.

**UNKNOWN:** Which content branch ran in the accepted stock-Notebook cycle,
whether its visible pixels were complete before map or produced by the Expose
callback, and which of the content/cache side effects are required by other
stock components.

## `blanket_image_window_bringup`

**FACT — target common library, high confidence:** The exported helper at
`0x864c` performs only this presentation sequence after argument validation:

1. obtain the X display through `blanket_image_screendisplay`;
2. call `XGetWindowAttributes` for the supplied window;
3. return without mapping if the reported `map_state` is not `IsUnmapped`;
4. select the configured input mask with `XSelectInput`;
5. call `XMapRaised`;
6. call `XFlush`;
7. record that window in the common blanket image context if needed.

It does not draw or copy image data, call a win-manager utility, call LIPC,
write persistent state, call an ioctl, access fbdev/HWTCON/MXCFB, request a
named E-Ink update, or call `XSync`. The only direct caller found in the
screensaver plugin is the normal unmapped-to-mapped branch at `0x3ea6`.

**FACT:** The helper is a narrow X11 map/raise boundary. It is not itself the
whole visible-replacement boundary, because `XMapRaised` can cause the plugin's
registered X11 Expose callback to run. That callback contains deferred Cairo
rendering and `win_mgr_utils_create_flash_trigger` calls.

**INFERENCE:** The smallest conceptual operation capable of withholding the
stock screensaver window is the transition immediately before
`blanket_image_window_bringup`, provided the plugin remains internally in the
unpresented state. Confidence is **medium-high** for X11 visibility and low for
the eventual physical-panel result.

## Exit symmetry

**FACT — target plugin, high confidence:** The `outOfScreenSaver` callback at
`0x533c` first emits a navigation metric and then calls the unmap helper at
`0x51e8`. That helper:

1. calls `blanket_image_window_teardown` only if plugin presentation state is
   nonzero;
2. clears both presentation state and the recorded content mode;
3. opens LIPC under the plugin service identity;
4. sends `unmap_screensaver`;
5. sends `nonDtcpScreensaverOut`;
6. closes the LIPC handle.

The two exit events are attempted even when presentation state was already
zero. The `exitingScreenSaver` callback at `0x2758` emits only its navigation
metric and does not hide/destroy the window or reset presentation state.

**FACT — target common library, high confidence:** The corresponding
`blanket_image_window_teardown` helper at `0x8840` validates its arguments,
obtains the X display, calls `XUnmapWindow`, obtains the display again, calls
`XFlush`, and updates the common blanket image context's current-window field.
It does not check current X11 map state and does not draw, call LIPC, call a
win-manager utility, persist files, or destroy the window.

**FACT:** Window destruction is separate. The plugin `deinit` path first calls
the normal unmap helper, then calls `blanket_image_window_destroy`, destroys
the Cairo surface and context, frees retained content state, and frees the
module state.

**INFERENCE:** Bringup and teardown are symmetric at the common-library X11
map/unmap level. The plugin adds asymmetric state and notification behavior
around them. A future gate placed before bringup and leaving presentation
state zero would cause normal `outOfScreenSaver` to skip XUnmapWindow while
still clearing mode and sending both exit events.

**UNKNOWN:** Whether teardown on an already-unmapped window is harmless on the
target, and whether wake clients require an actual XUnmap transition rather
than only the two LIPC events. Static evidence alone cannot establish runtime
safety.

## Side effects by phase

| Side effect | Timing | Evidence and limit |
| --- | --- | --- |
| navigation metrics | entry and both exit callbacks | Direct calls; bookkeeping |
| callback-argument message replacement | before presentation | `free`, `dcgettext`, `strdup`; in-memory state |
| household/profile and book-info LIPC reads | before presentation, applicable content branches | Content selection; no presentation acknowledgement |
| Cairo surface/context preparation | before presentation | In-memory rendering state |
| pending default/book-cover paint flags | before presentation | Cause later X11 repaint work after map |
| `/var/local/blanket/screensaver` directory and `last_ss` read/write | conditional rendering/content-selection paths, including a prerender-reached default path and repaint paths | Persistent content-rotation/cache state; exact branch in a given cycle **UNKNOWN** |
| `XSelectInput`, `XMapRaised`, `XFlush` | presentation helper | X11 visibility operation only |
| Cairo repaint and win-manager flash trigger | after map, in mapped-window Expose branches | Deferred rendering/presentation; physical panel-update owner still **UNKNOWN** |
| `bookCoverImpression` LIPC property update | book-cover rendering paths after book-cover metric construction | Content metric/notification; not generic to every entry |
| `XUnmapWindow`, `XFlush` | normal `outOfScreenSaver`, only if plugin state is nonzero | X11 teardown |
| `unmap_screensaver` and `nonDtcpScreensaverOut` events | normal unmap helper, including state-zero path | Wake/exit notification side effects |
| X11/Cairo/content destruction | module `deinit` only | Whole-module teardown, not normal wake |

No timer-creation call was found in the reviewed entry, X11 repaint, exit, or
deinit paths. The plugin has a `nanosleep` dynamic import, but no call to it was
found in the focused disassembly; this scoped result does not prove that the
module or its dependencies never delay work.

## Existing runtime control surfaces

**FACT — target common library, high confidence:** The generic blanket loader
accepts `load` and `unload` property operations, queues one loader event per
name, and reports loaded module names through its loader getter. `uiQuery` is
also registered by blanket, but no reviewed screensaver callback or common
loader path makes it a screensaver-window visibility control.

**FACT:** Whole-module unload is not equivalent to suppressing bringup. The
reviewed `blanket_loader_unload` sequence:

1. unsubscribes the module's LIPC callbacks;
2. invokes the module's `deinit` function;
3. removes and frees retained callback records;
4. frees the module name;
5. calls `dlclose`;
6. frees the module loader record.

It logs and continues through some unsubscribe/deinit failures, including a
diagnostic for callbacks that still exist. The screensaver deinitializer then
performs the broad teardown described above. A later load resolves the shared
object, registers callbacks, calls `init`, and creates fresh plugin window and
Cairo state.

**INFERENCE:** Unloading `screensaver` would remove entry and exit lifecycle
callbacks, destroy window/render state, and invoke exit notifications; it is
too broad to test whether only presentation may be omitted while preserving
all other plugin behavior. KOReader's use of unload/reload is evidence that the
generic control exists, not evidence that it preserves stock-Notebook
lifecycle and wake behavior.

## Candidate intervention boundaries

| Rank | Candidate | Lifecycle preserved | Expected display effect | Rollback / risk | Static decision |
| --- | --- | --- | --- | --- | --- |
| 1 | Gate the plugin immediately before bringup and keep its presentation state at 0 | Entry metrics, message selection, LIPC content reads, prerender, and normal exit notifications; post-map Expose effects would not occur | Screensaver X11 window should remain unmapped; physical E-Ink retention **UNKNOWN** | Conceptually local, but no existing runtime control was found; naïve helper suppression leaves state inconsistent and patching/hooking is disallowed for the first experiment | Strongest semantic boundary; not yet an executable experiment |
| 2 | Keep the plugin loaded but control only `blanket_screensaver` window/layer visibility through X11/Awesome | Potentially more plugin behavior than rank 1, depending on timing | Could hide/unmap the window, but may occur after a visible map/flash | No narrow stock interface was found; risks racing Expose and disagreeing with plugin state | Lower confidence; no approved control surface |
| 3 | Use blanket's whole-module `unload`/`load` | Removes most plugin lifecycle rather than preserving it | Likely prevents plugin entry presentation while unloaded | Existing high-level reversible control, but broad deinit/reinit, callback, exit-event, cache, and wake risks | Too broad for a presentation-only first test |

No candidate is approved for target execution.

## First reversible experiment gate

The static semantics are clear enough to reject a naïve bringup no-op and to
classify whole-module unload as over-broad. They are not sufficient to design
the required first state-changing experiment without binary patching.

The strongest candidate boundary is rank 1: preserve the entry callback and
prerender work, withhold the map transition, and leave the plugin state
coherently unpresented. The following evidence still blocks an experiment:

1. a stock, non-patching runtime control must be found that can withhold only
   map/layer visibility while keeping plugin and common-library state coherent;
2. the role of post-map Expose side effects must be separated into required
   lifecycle effects versus presentation-only work, especially flash-trigger,
   content-rotation/cache, and book-cover notification paths;
3. read-only runtime evidence must correlate the plugin window's map state (or
   an existing layer-active getter) with the visible replacement on this
   firmware;
4. a restore operation must be demonstrated to restore the original stock
   window/module state without reboot before it can be used as rollback;
5. the eventual one-variable trial must retain pre/post suspend statistics,
   kernel PM evidence, normal physical wake, Notebook restoration, and a
   bounded independent fail-safe.

If no non-patching narrow control exists, the next decision is not to patch the
binary automatically. It is whether a separately reviewed whole-module
unload/reload experiment is worth its broader scientific question and risk.
That experiment would not prove that bringup alone is dispensable.

## Decision

- **FACT:** `blanket_image_window_bringup` is a narrow X11 map/raise helper.
- **FACT:** normal plugin entry surrounds it with content, cache, LIPC, render,
  and internal-state work; mapping can trigger deferred Expose rendering.
- **INFERENCE:** a coherent pre-bringup gate is the narrowest promising StayInk
  presentation boundary.
- **UNKNOWN:** whether omitting that map preserves the physical page, whether
  post-map side effects are required, and whether an unmodified runtime control
  can express the gate.
- **Decision:** Phase 3E does not authorize the first reversible target
  experiment. Whole-module unload is too broad, and helper suppression would
  presently require prohibited patching/hooking or leave state inconsistent.
