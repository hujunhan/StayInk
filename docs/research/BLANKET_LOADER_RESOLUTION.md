# Phase 3C — Blanket loader/module resolution

## Scope and question

This is offline, read-only static research on owner-supplied copies of the
target `/usr/sbin/blanket`, `/usr/lib/libblanket.so.1.0`, and
`/usr/lib/blanket/screensaver.so.1.0`. The target is the UI-identified Kindle
Scribe on firmware 5.19.5; its generation and physical model number remain
**UNKNOWN**. No target interaction occurred, and no copied artifact was run.

The concrete question is how the loader expression containing `screensaver`
is resolved, which exact artifact implements it, and what that artifact does
at the presentation boundary. The reviewed parser, module loader, and plugin
now answer the filename/ABI and content/window ownership parts; physical panel
update submission remains outside the demonstrated boundary.

## Focused launcher call path

**FACT — target binary, high confidence (OBS-012 / FACT-024):** The launcher's
`getopt` option string is `dvt:h`. When option `t` is selected, the launcher
loads the `optarg` pointer and retains it without performing a visible copy,
filename conversion, symbol construction, or directory lookup. After a
successful `blanket_init`, it calls the imported function as the equivalent of:

```text
blanket_event_parse_loader(blanket_context, t_optarg, 0)
```

The call occurs at virtual address `0x1dde`; failure produces the embedded
`PARSE_LOADERS_FAILED` diagnostic. The parser body is not present in the
launcher.

**FACT — target binary, high confidence:** The launcher has no undefined
import named `dlopen`, `dlsym`, `g_module_open`, `g_module_symbol`, or another
obvious module-opening API. It contains no loader directory, `.so` filename
template, loader symbol prefix, or screensaver-specific filename in the
reviewed dynamic metadata and strings. This negative result applies only to
the copied launcher.

**UNKNOWN:** How `blanket_event_parse_loader()` tokenizes or stores the loader
expression, and whether it resolves a fixed object, function table, shared
library, another process/window, or another mechanism. Those operations occur
beyond the launcher's imported-call boundary.

That launcher-only unknown is resolved by the supplied library below.

## Library parser and event dispatch

**FACT — target library, high confidence (OBS-014 / FACT-026):** The copied
library exports `blanket_event_parse_loader`. The function copies the supplied
loader expression, iterates token results from `strtok_r`, and enqueues one
loader event per result with `blanket_event_enqueue_loader`. It then wakes the
blanket main thread. It does not open a module in the caller's thread. The
reviewed function occupies virtual addresses `0x73a4`–`0x74f7`.

**FACT — target library, high confidence:**
`blanket_event_handler_loader` dispatches queued load events to
`blanket_loader_load` and queued unload events to `blanket_loader_unload`.
After a successful operation it emits blanket system events through
`blanket_loader_emit_system_events`. This establishes a queue-mediated loader
lifecycle rather than an internal fixed function selected directly by the
launcher. The reviewed handler occupies `0x6efc`–`0x70bf`.

**FACT — target library, high confidence:** A successfully loaded module is
stored in blanket's module list under a duplicated copy of its loader name.
The reviewed path checks that list before loading another module with the same
name.

## Dynamic-library boundary

**FACT — target binary, high confidence:** The executable uses
`/lib/ld-linux-armhf.so.3` as its ELF interpreter and contains a `DT_NEEDED`
entry for the SONAME `libblanket.so.1.0`. It contains no `DT_RPATH` or
`DT_RUNPATH`, and it does not embed an absolute path for that library.

**FACT — target binary, high confidence:** The following blanket-named
functions are undefined dynamic symbols with PLT/JUMP_SLOT relocations:

| Rank | Imported function | Demonstrated launcher use |
| --- | --- | --- |
| 1 | `blanket_event_parse_loader` | Receives the `-t` loader expression after blanket initialization |
| 2 | `blanket_init` | Initializes the blanket context before loader parsing and exposes the X11 connection/wakeup-pipe state used by the loop |
| 3 | `blanket_event_dispatch` | Dispatches each dequeued blanket event |
| 4 | `blanket_event_enqueue_x11` | Converts/queues an event obtained through `XNextEvent` |
| 5 | `blanket_event_dequeue` | Removes the next event from the blanket queue |
| 6 | `blanket_event_is_Q_empty` | Controls queue draining |
| 7 | `blanket_event_destroy` | Releases a dispatched event |
| 8 | `blanket_deinit` | Tears down the blanket context |

There are no launcher imports specifically named for screensaver presentation,
window creation/mapping, loader load/unload, or show/hide. Its only direct X11
imports are `XConnectionNumber`, `XPending`, and `XNextEvent`; it does not
directly import `XCreateWindow`, `XMapWindow`, `XUnmapWindow`, or another
window-mutation function.

**FACT — target library, high confidence:** The supplied library has SONAME
`libblanket.so.1.0` and exports the launcher's blanket-named imports, including
`blanket_event_parse_loader`, `blanket_init`, event queue/dispatch functions,
and teardown. This closes the earlier provider uncertainty for the copied
artifact pair, while not proving which mapped object satisfied the symbols in
an unobserved runtime process.

## Module resolution and ABI

**FACT — target library, high confidence:**
`blanket_loader_get_module_path_if_exists` opens the fixed directory
`/usr/lib/blanket`, formats the requested loader name with `%s.so`, compares
that exact filename against directory entries, and, on a match, constructs the
full path with `%s/%s` (`0xa98c`–`0xab53`). For the established loader token `screensaver`, the
only path constructed by this reviewed function is:

```text
/usr/lib/blanket/screensaver.so
```

**FACT — target pathname metadata, high confidence (OBS-015 / FACT-027):** On
this Scribe/5.19.5 filesystem, that exact path exists as a symbolic link:

```text
/usr/lib/blanket/screensaver.so -> screensaver.so.1.0
```

Its canonical target is `/usr/lib/blanket/screensaver.so.1.0`.

**FACT — target library, high confidence:** `blanket_loader_load` obtains that
path, calls `dlopen(path, 1)`, resolves symbols through `dlsym`, and closes the
handle on failure or unload (`0xbc78`–`0xbf93`). The numeric mode is compatible with
`RTLD_LAZY`; the formal mode interpretation is supplied by the target libc and
is not needed to establish that this is a dynamically loaded shared object.

**FACT — target library, high confidence:** The module loader resolves six
symbols from every module: `init`, `deinit`, `lipcCallbacks`,
`lipcCallbackNum`, `x11Callback`, and `kiwiCallback`. It registers the callback
set and then calls the resolved `init` function. `blankwindow` is a named
special case for which callback registration is skipped before initialization.
The six-symbol resolver occupies `0xb500`–`0xb64f`. This is a
function/data-symbol module ABI, not another process or merely a window name.

**FACT — scoped negative result:** No alternative loader directory, filename
prefix, or symbol-name derivation was found in the reviewed parser and directly
reached load path. This does not rule out delegation performed inside an
individual plugin after its `init` or callbacks run.

## X11, Awesome, and display attribution

**FACT:** The launcher consumes X11 events and hands them to blanket's imported
queue/dispatch layer. The module ABI includes `x11Callback` and
`kiwiCallback`, placing loaded modules in the X11/event-dispatch path.

**FACT — target library, high confidence:** `libblanket.so.1.0` provides common
image/window helpers. Its `blanket_image_get_window` path can construct a
Kindle screensaver window name with
`win_mgr_utils_new_screen_saver_name`, creates an X11 window, and sets window
metadata. Its bring-up/teardown helpers call `XMapRaised` and `XUnmapWindow`.
The relevant helper bodies begin at `0x9c1c`, `0x864c`, and `0x8840`. This
demonstrates a library-level X11/Awesome-compatible presentation capability.

**FACT — target library, high confidence:** The same library also exports
fbdev/image helpers, contains `/dev/fb0`, and has demonstrated calls to
`open`, `ioctl`, Cairo, and rendering functions. Direct display capability
therefore exists downstream of the thin launcher in the common library.

**FACT — target plugin, high confidence (OBS-016 / FACT-028):** The copied
`screensaver.so.1.0` is a stripped ARM EABI5 shared object with SONAME
`screensaver.so.1.0`. It exports all six symbols required by the loader ABI:
`init`, `deinit`, `lipcCallbacks`, `lipcCallbackNum`, `x11Callback`, and
`kiwiCallback`. The read-only `lipcCallbackNum` value is 13.

**FACT — target plugin, high confidence:** The callback table directly binds
the plugin to these relevant lifecycle events:

| Source | Event | Static callback relationship |
| --- | --- | --- |
| `com.lab126.powerd` | `goingToScreenSaver` | Entry/presentation callback |
| `com.lab126.powerd` | `outOfScreenSaver` | Exit/unmap callback |
| `com.lab126.powerd` | `exitingScreenSaver` | Exit-side callback |
| `com.lab126.powerd` | `userShutdown` | Shutdown-side callback |
| `com.lab126.powerd` | `outOfShutdown` | Exit/unmap-side callback |
| `com.lab126.hal.screensaver` | `goingToScreenSaver` | Same entry callback |
| `com.lab126.authenticator` | `goingToScreenSaver`, `outOfScreenSaver` | Same entry/exit callbacks |
| `com.lab126.household` | `profileGoingToSwitch` | Profile-switch callback |
| `com.lab126.test.blanket` | entry/exit/shutdown variants | Test-source equivalents |

This establishes direct event consumption by the plugin; it does not define
the payloads or prove which alternate source is active in a normal target
cycle.

**FACT — target plugin, high confidence:** `init` obtains an X11 window named
`blanket_screensaver` through `blanket_image_get_window`, queries the common
display/visual/size helpers, and creates a Cairo Xlib surface and context.
`deinit` destroys the blanket window and Cairo objects. The
`goingToScreenSaver` callback reaches module prerender/render logic and then
`blanket_image_window_bringup`; the `outOfScreenSaver` path reaches
`blanket_image_window_teardown`. The plugin also registers an X11 callback
whose body contains a repaint path.

**FACT — target plugin, high confidence:** Embedded source/function names and
directly reached imports identify content work including
`module_screensaver_renderDefaultScreenSaver`,
`module_screensaver_renderBookCover`, `module_screensaver_prerender`,
`module_screensaver_repaint`, `blanket_image_get_asset_name`, and
`blanket_image_renderer`. The plugin therefore prepares/renders stock
screensaver content as well as controlling the X11 window; it is not merely a
lifecycle coordinator.

**FACT — target plugin, scoped:** The plugin also imports
`LipcCreateAndSendEvent`, `LipcSetStringProperty`, `open`, `write`, and
`g_mkdir_with_parents`, and contains paths under
`/var/local/blanket/screensaver` plus diagnostics for `unmap_screensaver` and
`nonDtcpScreensaverOut`. Static presence does not define every call's runtime
effect, but it shows that the module boundary cannot yet be treated as a
side-effect-free window toggle.

**FACT:** Generic SRC-003 documentation describes an Awesome `SS` layer and
screensaver roles. The target plugin's `blanket_screensaver` X11 window,
`blanket_image_window_bringup`/teardown calls, and
`win_mgr_utils_create_flash_trigger` import provide target-specific evidence
for X11/window-manager involvement. They do not prove which Awesome rule or
layer is selected at runtime.

**FACT — scoped negative result:** The plugin imports Cairo, X11-facing blanket
window/image helpers, Lab126 graphics, and window-manager utilities. It does
not directly import `ioctl`, a framebuffer/HWTCON/MXCFB interface, or an E-Ink
update API, and no `/dev/fb` string was found in this plugin. The common
library's `blanket_image_renderer` path performs Cairo image composition; the
reviewed renderer body does not itself submit a framebuffer ioctl. The common
library separately has `blanket_image_set_fbdev_mode`, which can use `ioctl`,
but no direct call from this plugin to that helper was found.

**UNKNOWN:** Which X11/Awesome, Lab126 graphics, or lower display component
turns the mapped/repainted window into an HWTCON panel update; whether the map,
flash trigger, damage handling, or another action is the decisive refresh
request; and whether the physical panel contents would remain unchanged if
only plugin presentation were omitted.

## Plugin artifact result

**Observed result (OBS-013 / FACT-025):** On the target Scribe running firmware
5.19.5, `/usr/lib/libblanket.so.1.0` exists as a regular executable file owned
by root, with observed size 76,160 bytes. `readlink -f` resolved it to the same
pathname, so the observed path is not a symlink to a different target.

The library and plugin pathname gates are complete. Do not repeat them or
broaden the filesystem query. The owner supplied the exact canonical artifact:

```text
/usr/lib/blanket/screensaver.so.1.0
```

The owner completed the previously reviewed **READ_ONLY** metadata query and
supplied:

```text
lrwxrwxrwx 1 root root 18 Jun 17 02:49 /usr/lib/blanket/screensaver.so -> screensaver.so.1.0
/usr/lib/blanket/screensaver.so.1.0
```

- **FACT:** the expected plugin exists and uses a versioned shared-object
  target.
- **FACT (OBS-016 / FACT-028):** the copy is a 34,660-byte ARM EABI5 shared
  object with SHA-256
  `864a9987ea0c556f7e837d7433c265f862845973444cc74aeba3674ed77acf4e`,
  SONAME `screensaver.so.1.0`, and the expected six-symbol blanket module ABI.
- **FACT:** focused inspection resolved its lifecycle subscriptions,
  initialization, content-rendering, X11 bring-up/repaint, and teardown paths.
- **UNKNOWN:** runtime mapping in a particular cycle and downstream physical
  panel-refresh ownership.

No additional target filesystem metadata is needed for loader/module
resolution. The owner-supplied proprietary binaries remain evidence artifacts
only and must not be committed.

## StayInk intervention relevance

**HYPOTHESIS:** Powerd and blanket could remain running while only the
`screensaver` presentation loader's visible action is skipped or hidden,
leaving the existing Notebook window/panel contents visible while the normal
`readyToSuspend` and kernel `mem` path proceeds.

Supporting facts now include: powerd's lifecycle is separable from kernel
suspend in Phase 2; blanket resolves `screensaver` to the inspected plugin;
that plugin consumes `goingToScreenSaver`, renders/prepares screensaver
content, and brings up the `blanket_screensaver` X11 window; and KOReader has a
third-party path that unloads/reloads a blanket screensaver module.

Unsupported assumptions remain substantial: the loader may gate more than
presentation; it may own wake restoration, lifecycle notifications, or
content-rotation state; unloading, skipping, or hiding it may affect
Awesome/window state; a downstream component still owns the physical panel
update; and panel contents may not survive a normal suspend.

**Decision:** The `goingToScreenSaver` callback's render-to-window-bring-up
path inside `screensaver.so.1.0` is the strongest demonstrated presentation
boundary. `libblanket.so.1.0` is sufficient to resolve/load the plugin and
supplies its common X11/image helpers; no additional plugin was identified in
the directly reached presentation path. This is not yet a controllable StayInk
boundary. Intervention design still requires verified runtime control and
recovery semantics, evidence that bypassing only visible presentation
preserves readiness/kernel suspend and normal wake, and separation of the
window action from downstream panel-refresh ownership.

Phase 3E refines this result in
`docs/research/PRESENTATION_BOUNDARY_SEMANTICS.md`: the common bringup helper is
a narrow X11 map/raise operation, but its caller immediately records the
plugin as presented and the resulting Expose path may perform deferred render
and flash-trigger work. Whole-module unload is correspondingly too broad, and
no coherent non-patching presentation-only runtime control is yet established.
