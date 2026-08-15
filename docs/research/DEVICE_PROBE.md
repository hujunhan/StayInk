# Kindle Scribe Phase 2A Device Probe Specification

Status: COMPLETE; executed manually with read-only observations; no executable probe created
Designed: 2026-08-14; completed: 2026-08-15 UTC
Target model / generation: Kindle Scribe / **UNKNOWN**
Target firmware / build: 5.19.5 / detailed build **UNKNOWN**
Execution environment and privileges: Véra/KPM; BusyBox shell as root with broad effective capabilities; KOReader publisher active

## Scope and decision boundary

The concrete question for Phase 2A is: **Which read-only observables are actually present on the target Scribe, and are they sufficient to design a separate suspend/display trace?** This matters because StayInk must preserve the displayed image while still allowing genuine low-power suspend and normal wake.

This probe is environment discovery only. Its commands do not press or inject the power button, request or control suspend, wake the device, trace a controlled transition, change a LIPC property, access framebuffer pixels, or alter a service. It therefore cannot identify the sleep-screen renderer, establish event ordering, prove electrical low-power residency, or test display persistence.

Important terms:

- **READ_ONLY**: the command requests or reads existing information and has no write/control argument. Ordinary observer effects still exist: a remote shell creates processes, network traffic, and possibly volatile audit/log entries. These effects make Phase 2A unsuitable for power measurement.
- **UNCERTAIN**: the target implementation, breadth, privacy impact, or runtime effect has not been established well enough for the first interaction.
- **STATE_CHANGING**: the operation intentionally changes device, service, power, display, logging, or filesystem state.

Only the commands explicitly marked **READ_ONLY** below are approved by this specification. Run them manually, while the Kindle is already awake, and capture stdout and stderr on the development machine. Do not redirect output to a Kindle file. If a path or command is absent, record the error and continue; do not install a replacement or escalate privileges. Where one observation contains a multiline compound command, every line is covered by that observation's purpose and classification; no unlisted fallback is implied.

### Preconditions, success, and stop criteria

- Use only an already-installed, already-enabled jailbreak shell facility. If access would require enabling a service, changing a setting, installing a key/package, remounting, or modifying startup, stop; this specification does not authorize that setup.
- Keep the device awake through ordinary prior use. Do not use a power/suspend command or inject input to prepare the session.
- Capture output on the development machine. The recovery/rollback for the shell's transient observer effect is simply to exit the session; this probe creates no approved device artifact to clean up.
- Stage 1 succeeds if it records or explicitly fails to obtain the UI identity, firmware, kernel/architecture, execution identity, mount policy, and tool paths. An error is valid evidence.
- Stage 2 succeeds if it produces a bounded availability map for processes, power/wake, input, framebuffer metadata, named IPC publishers/getters, and log surfaces. It need not return data for every path.
- Stop immediately if a command differs from the documented implementation, prompts to alter configuration, exposes notebook/document content, begins a power transition, or would require a write or privilege escalation. Preserve the already-captured development-machine transcript and make no recovery change on the Kindle.

## Evidence basis for the narrow LIPC reads

**FACT:** Within the reviewed KindleModding documentation, `lipc-get-prop` issues a property get, while `lipc-set-prop` is the write operation. The same pinned source labels `com.lab126.powerd` `state` and `status`, and `com.lab126.winmgr` `isScreenSaverLayerWindowActive`, as readable. Source: SRC-003, `KindleModding/kindlemodding.github.io` @ `cf4457c4808636dac5e06d2f2761cb4632efe66d`, `content/kindle-apps-and-services/_index.md`, `com.lab126.powerd.md`, and `com.lab126.winmgr.md`; model/firmware applicability **UNKNOWN**; confidence medium; relevance is selection of narrow getter-only observations.

**FACT:** KOReader reads `com.lab126.powerd state` through its LIPC getter. Source: SRC-004, `koreader/koreader` @ `2d505d1ea32ac875c18ed07b07d8bef78ae6954c`, `frontend/device/kindle/powerd.lua` :: `KindlePowerD:getState`; generic Kindle integration / firmware **UNKNOWN**; confidence high for KOReader behavior, not for stock Scribe semantics; relevance is evidence that the named interface has been used observationally.

**FACT:** SRC-003 documents a system-bus `GetConnectionUnixProcessID` query for mapping a registered service name to its current process ID. This is a bus-daemon metadata query, not proof that the returned process performs a particular display action. Same repository/commit, `_index.md` :: “Identifying what process owns a service”; model/firmware applicability **UNKNOWN**; confidence medium.

No external source code is copied or linked into StayInk. These sources validate only the proposed observation form; they do not establish Scribe availability or behavior.

## Before any shell connection: manual UI record

### Observation UI-1 — Target and trial context

**COMMAND OR OBSERVATION:** With the Kindle awake, manually record the marketed model name/generation, physical model number if visible without disassembly, firmware version/build shown by the stock UI, current date/time, battery percentage and charging state, Wi-Fi state, orientation, whether a passcode and Special Offers/ads are enabled, whether a magnetic cover is fitted, and the installed jailbreak/launcher/package-manager names and versions if their normal UI exposes them. Close notebooks and personal documents, remove the cover, and leave a non-sensitive stock Settings page visible. Do not photograph or copy the full serial number, MAC address, account name, document title, or notebook content.

**PURPOSE:** Establish the support-matrix row and privacy-safe baseline before a shell changes the process/network environment.

**EXPECTED INFORMATION:** Exact target identity and UI variants that may select different display or wake paths.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** It uses only non-persistent manual observation through ordinary UI navigation. Navigation temporarily changes the visible UI state, but it does not edit notebook data, system files, or power settings.

**PORTABILITY / AVAILABILITY RISK:** UI labels and menu locations vary by firmware. A marketed generation label may be ambiguous; record the wording rather than interpreting it.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows UNK-008. Passcode, ads, cover, and UI variants identify later UNK-001/UNK-006 cases but do not resolve them.

## Stage 1: smallest useful first shell interaction

Run Stage 1 manually and stop to review/redact its output before Stage 2. The multiline loops are shell built-ins plus simple reads; paste them exactly and do not replace `<` with `>`.

### Observation S1-1 — Timestamp

**COMMAND OR OBSERVATION:** `date -u '+%Y-%m-%dT%H:%M:%SZ'`

**PURPOSE:** Correlate the captured output with the manual UI record and any pre-existing logs.

**EXPECTED INFORMATION:** Device wall-clock time in UTC.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `date` without a setting argument reads the current clock.

**PORTABILITY / AVAILABILITY RISK:** Low; formatting support may differ. An inaccurate device clock limits correlation.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Provides context for all unknowns; resolves none by itself.

### Observation S1-2 — Firmware string

**COMMAND OR OBSERVATION:** `cat /etc/prettyversion.txt`

**PURPOSE:** Identify the installed firmware/build from an existing version file.

**EXPECTED INFORMATION:** Amazon firmware or build text, if the file exists.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `cat` opens the fixed file for reading and has no output redirection.

**PORTABILITY / AVAILABILITY RISK:** Medium; the path may be absent or its format may change. Do not search the entire root filesystem if absent.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows UNK-008.

### Observation S1-3 — Kernel and architecture

**COMMAND OR OBSERVATION:** `uname -a`

**PURPOSE:** Identify kernel release, build, machine architecture, and hostname context.

**EXPECTED INFORMATION:** Kernel name/release/build and architecture.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `uname` reads kernel identity through a standard system call.

**PORTABILITY / AVAILABILITY RISK:** Low. Redact a non-generic hostname if one appears.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows UNK-008 and informs availability of later UNK-002 suspend observables.

### Observation S1-4 — CPU/platform record

**COMMAND OR OBSERVATION:** `cat /proc/cpuinfo`

**PURPOSE:** Record the kernel-exported CPU and platform description without assuming the community MTK mapping applies.

**EXPECTED INFORMATION:** CPU architecture/features and possibly hardware/platform labels.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `/proc/cpuinfo` is generated kernel metadata opened only for reading.

**PORTABILITY / AVAILABILITY RISK:** Medium; fields are architecture- and kernel-specific. Redact any full serial or unique hardware identifier before copying the output.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows UNK-008; can check whether FACT-006's community classification is consistent with the target, but cannot confirm stock display behavior.

### Observation S1-5 — Device-tree and SoC labels

**COMMAND OR OBSERVATION:**

```sh
for f in /sys/firmware/devicetree/base/model /sys/devices/soc0/machine /sys/devices/soc0/family /sys/devices/soc0/soc_id /sys/devices/soc0/revision; do
    if [ -r "$f" ]; then
        printf '%s: ' "$f"
        tr -d '\000' < "$f"
        printf '\n'
    fi
done
```

**PURPOSE:** Obtain non-assumed platform/model labels from fixed kernel metadata paths.

**EXPECTED INFORMATION:** Device-tree model and available SoC family/ID/revision strings.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** The loop tests readability and reads only the named metadata files. `tr` removes device-tree NUL terminators from captured output.

**PORTABILITY / AVAILABILITY RISK:** High; some or all paths may be absent, permissions may deny access, and labels may be vendor codenames rather than definitive product identity.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows UNK-008 and tests consistency with FACT-006 without generalizing from another Kindle.

### Observation S1-6 — Effective identity and capabilities

**COMMAND OR OBSERVATION:**

```sh
id
grep -E '^(Uid|Gid|Groups|CapInh|CapPrm|CapEff|CapBnd|CapAmb|NoNewPrivs|Seccomp):' /proc/$$/status
readlink -f /proc/$$/exe
```

**PURPOSE:** Determine the account, groups, Linux capability masks, confinement indicators, and actual shell executable supplied by the jailbreak environment.

**EXPECTED INFORMATION:** Whether the session is root or restricted and which capability/confinement indicators the kernel exposes.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `id`, `grep` of the current shell's `/proc/$$/status`, and `readlink` only inspect the current shell/session metadata.

**PORTABILITY / AVAILABILITY RISK:** Medium; older kernels may omit capability, seccomp, or `readlink -f` details. Capability masks require offline interpretation and do not prove every operation is permitted.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Informs UNK-007 and whether later read-only observables are accessible; resolves no power/display unknown.

### Observation S1-7 — Mount policy

**COMMAND OR OBSERVATION:** `cat /proc/mounts`

**PURPOSE:** Identify filesystem types and current read-only/read-write mount flags without mounting or remounting anything.

**EXPECTED INFORMATION:** Mount points and options for rootfs, `/var/local`, `/mnt/us`, debugfs, procfs, and sysfs when present.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `/proc/mounts` is generated mount metadata opened only for reading.

**PORTABILITY / AVAILABILITY RISK:** Low. Redact unusual user-supplied volume names if present. A writable mount does not authorize writing to it.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Informs UNK-007 and availability of later UNK-002 kernel observables.

### Observation S1-8 — Required tool availability

**COMMAND OR OBSERVATION:**

```sh
for c in ps lipc-get-prop lipc-probe lipc-wait-event dbus-send dbus-monitor dmesg logread showlog fbset; do
    printf '%s: ' "$c"
    command -v "$c" || printf 'NOT FOUND\n'
done
```

**PURPOSE:** Discover whether the existing image/jailbreak provides candidate observation tools without invoking them or installing anything.

**EXPECTED INFORMATION:** Resolved executable paths or `NOT FOUND`.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** The shell's `command -v` performs name/path lookup and does not execute the named tools.

**PORTABILITY / AVAILABILITY RISK:** Low for the loop; aliases/functions could be reported instead of files. Tool presence does not establish safe semantics.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Determines which later observations could address UNK-001 through UNK-006; resolves none by itself.

## Stage 2: approved awake-state inventory

Proceed only after Stage 1 output is reviewed and redacted. A missing command or permission is a result, not a reason to use `sudo`, `su`, package installation, or a write operation.

### Observation S2-1 — Running processes

**COMMAND OR OBSERVATION:** Run `ps w`. If that syntax is rejected, record the error and run `ps` once.

**PURPOSE:** Inventory current process names/arguments and look specifically for powerd, blanket, pillow, window-manager, GUI, display, and jailbreak components.

**EXPECTED INFORMATION:** A snapshot of process IDs and names, with command lines if supported.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `ps` reads process-table metadata and sends no signals.

**PORTABILITY / AVAILABILITY RISK:** Medium; BusyBox/procps options and truncation differ. Arguments can contain identifiers or personal filenames; redact them before copying. A process name is an observation, not proof that it owns the sleep refresh.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Provides candidates for UNK-001 and context for UNK-006/UNK-007, but does not attribute display operations.

### Observation S2-2 — Advertised power states

**COMMAND OR OBSERVATION:**

```sh
for f in /sys/power/state /sys/power/mem_sleep /sys/power/wakeup_count; do
    if [ -r "$f" ]; then
        printf '%s: ' "$f"
        cat "$f"
    fi
done
```

**PURPOSE:** Discover kernel-advertised sleep-state names, selected/available `mem` variants, and whether a wakeup counter is exposed.

**EXPECTED INFORMATION:** Strings such as supported state names and bracketed `mem_sleep` selection, plus a current counter value if available.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** Each fixed sysfs attribute is opened only for reading; nothing is written to `/sys/power`.

**PORTABILITY / AVAILABILITY RISK:** High; attributes and terminology vary by kernel. An advertised state is not evidence that the Scribe entered it, and `wakeup_count` alone is not a suspend counter.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows candidate observables for UNK-002; does not prove genuine suspend.

### Observation S2-3 — Suspend-statistics and tracepoint availability

**COMMAND OR OBSERVATION:**

```sh
ls -la /sys/power/suspend_stats /sys/kernel/debug/suspend_stats 2>&1
ls -la /sys/kernel/debug/tracing/events/power /sys/kernel/debug/tracing/events/power/suspend_resume 2>&1
```

**PURPOSE:** Determine whether already-exposed kernel suspend counters or power tracepoint metadata exist and are readable.

**EXPECTED INFORMATION:** Directory/file metadata or explicit absent/permission errors. No tracing is enabled.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `ls` reads directory and inode metadata only. It does not write trace controls or power-state files.

**PORTABILITY / AVAILABILITY RISK:** High; paths differ by kernel and debugfs may be absent or inaccessible. Existence does not establish semantics or permission to enable tracing.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows possible independent evidence for UNK-002.

### Observation S2-4 — Wake-source inventory

**COMMAND OR OBSERVATION:**

```sh
if [ -r /sys/kernel/debug/wakeup_sources ]; then
    cat /sys/kernel/debug/wakeup_sources
fi
for f in /sys/class/wakeup/wakeup*/name /sys/class/wakeup/wakeup*/device/power/wakeup; do
    if [ -r "$f" ]; then
        printf '%s: ' "$f"
        cat "$f"
    fi
done
```

**PURPOSE:** Inventory kernel-exposed wakeup-source names/counters and wake-enable metadata without changing any source.

**EXPECTED INFORMATION:** Candidate button, hall/cover, RTC, touch/stylus, USB, network, or SoC wake sources when the kernel exposes them.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** Only fixed debugfs/sysfs metadata is opened for reading; no `power/wakeup` attribute is written.

**PORTABILITY / AVAILABILITY RISK:** High; paths, names, counters, and permissions are platform-specific. A listed source does not prove it is a normal stock wake path or that a future intervention affects it.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Identifies later test cases for UNK-006 and possible evidence sources for UNK-002.

### Observation S2-5 — Input topology

**COMMAND OR OBSERVATION:** `cat /proc/bus/input/devices`

**PURPOSE:** Record registered input device names/handlers that may correspond to power button, hall sensor/cover, touch, and stylus paths.

**EXPECTED INFORMATION:** Kernel input-device descriptors and handler mappings.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** It reads generated input metadata and does not open, grab, or inject into any `/dev/input/event*` device.

**PORTABILITY / AVAILABILITY RISK:** Medium; labels can be generic or absent and do not establish wake capability.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Identifies later button/cover/stylus cases for UNK-006.

### Observation S2-6 — Framebuffer metadata

**COMMAND OR OBSERVATION:**

```sh
ls -l /dev/fb0 /sys/class/graphics/fb0 /sys/class/graphics/fb0/device 2>&1
for f in /sys/class/graphics/fb0/name /sys/class/graphics/fb0/modes /sys/class/graphics/fb0/virtual_size /sys/class/graphics/fb0/bits_per_pixel /sys/class/graphics/fb0/stride; do
    if [ -r "$f" ]; then
        printf '%s: ' "$f"
        cat "$f"
    fi
done
readlink -f /sys/class/graphics/fb0/device
```

**PURPOSE:** Identify framebuffer device presence, driver/device linkage, dimensions, modes, pixel depth, and stride without reading pixel memory or issuing a refresh ioctl.

**EXPECTED INFORMATION:** Device-node metadata and available framebuffer attributes.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `ls`, `cat` of fixed sysfs metadata, and `readlink` inspect metadata only. `/dev/fb0` itself is never opened for pixel access.

**PORTABILITY / AVAILABILITY RISK:** High; attributes can be absent and symlink targets can use vendor-specific names. Metadata cannot show which buffer is scanned out or what is visible on the physical panel.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows the interface portion of UNK-001/UNK-004 and checks consistency with FACT-006; it cannot establish panel/framebuffer persistence.

### Observation S2-7 — Registered system-bus names

**COMMAND OR OBSERVATION:**

```sh
dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.ListNames
```

Run only if Stage 1 found `dbus-send`.

**PURPOSE:** List currently registered system-bus names to determine whether the Phase 1 LIPC publishers are present.

**EXPECTED INFORMATION:** Bus names including any registered `com.lab126.powerd`, `com.lab126.blanket`, `com.lab126.pillow`, and `com.lab126.winmgr` services.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `ListNames` is a bus-daemon metadata query with no service-control or property-write argument.

**PORTABILITY / AVAILABILITY RISK:** Medium-high; `dbus-send` or the expected bus may be absent/restricted. Registered names do not enumerate LIPC properties/events and do not prove component behavior.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Confirms candidate interfaces relevant to UNK-001/UNK-003/UNK-006 without resolving them.

### Observation S2-8 — Owners of four candidate LIPC publishers

**COMMAND OR OBSERVATION:**

```sh
for s in com.lab126.powerd com.lab126.blanket com.lab126.pillow com.lab126.winmgr; do
    printf '\n%s\n' "$s"
    dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.GetConnectionUnixProcessID string:"$s"
done
```

Run only if Stage 1 found `dbus-send`. Cross-reference returned PIDs manually with Observation S2-1.

**PURPOSE:** Map known candidate publisher names to current PIDs/processes without inspecting or controlling those processes.

**EXPECTED INFORMATION:** A PID for each registered name or a clear not-owned/error response.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** The query asks the bus daemon for connection metadata; it does not call the candidate service or send it a control value.

**PORTABILITY / AVAILABILITY RISK:** High; publishers may be absent, renamed, proxied, or owned by a shared process. Ownership of a publisher is not proof of stock sleep-screen replacement.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Narrows process/service candidates for UNK-001 and prepares UNK-003 tracing.

### Observation S2-8A — Targeted service-owner process identities

Use this follow-up only when Observation S2-1 does not expose the system processes. It is one manually pasted compound observation, not an executable probe file.

**COMMAND OR OBSERVATION:**

```sh
for s in com.lab126.powerd com.lab126.blanket com.lab126.pillow com.lab126.winmgr com.lab126.KPPMainApp.ScreenSaverListener com.github.koreader.kindlepowerd; do
    printf '\nSERVICE: %s\n' "$s"
    reply="$(dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.GetConnectionUnixProcessID string:"$s" 2>&1)"
    case "$reply" in
        *"uint32 "*) pid="${reply##*uint32 }" ;;
        *)
            printf 'owner_pid: UNAVAILABLE\n%s\n' "$reply"
            continue
            ;;
    esac
    case "$pid" in
        ''|*[!0-9]*)
            printf 'owner_pid: INVALID (%s)\n' "$pid"
            continue
            ;;
    esac
    printf 'owner_pid: %s\n' "$pid"
    printf 'exe: '
    readlink -f "/proc/$pid/exe" || printf 'UNAVAILABLE\n'
    printf 'comm: '
    cat "/proc/$pid/comm" || printf 'UNAVAILABLE\n'
    printf 'cmdline: '
    if [ -r "/proc/$pid/cmdline" ]; then
        tr '\000' ' ' < "/proc/$pid/cmdline"
        printf '\n'
    else
        printf 'UNAVAILABLE\n'
    fi
    grep -E '^(Name|PPid|Uid|Gid):' "/proc/$pid/status" || printf 'status fields: UNAVAILABLE\n'
done
```

**PURPOSE:** For only the six named publishers, obtain the current owner PID and immediately identify that process through narrowly selected procfs metadata. This includes the KOReader-named publisher as a potential third-party confounder; nothing is disabled or controlled.

**EXPECTED INFORMATION:** For each currently registered service: owner PID, executable path, kernel process name, NUL-separated command arguments rendered as spaces, parent PID, and real/effective/saved/filesystem UID and GID values. An unregistered service, exited process, permission failure, or empty command line is recorded rather than expanded into broader discovery.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** The already-reviewed D-Bus call queries bus-daemon ownership metadata. Shell variables, `case`, tests, and `printf` affect only the temporary shell process. `readlink`, `cat`, `tr`, and `grep` open only `/proc/<PID>/exe`, `comm`, `cmdline`, and `status` for reading. The command does not enumerate `/proc`, inspect `environ`, memory, file descriptors, or open-file contents, and sends no signal or service request.

**PORTABILITY / AVAILABILITY RISK:** Medium. The target has already demonstrated the required BusyBox shell operations and tools, but a service can exit or change owner between the D-Bus reply and procfs reads. `comm` and `Name` can be truncated, `cmdline` can be empty, and `/proc/<PID>/exe` can be inaccessible or marked deleted. Treat missing or inconsistent fields as an unresolved snapshot; do not retry through broad enumeration or inspect additional procfs paths.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Closes the Phase 2A process-inventory gap and distinguishes stock-named publisher owners from the KOReader-named confounder. It narrows candidates for UNK-001 and later UNK-003 tracing, but service ownership and executable identity still do not prove which component performs the sleep-screen framebuffer write or panel refresh.

### Observation S2-9 — Narrow powerd state getter

**COMMAND OR OBSERVATION:**

```sh
lipc-get-prop com.lab126.powerd state
```

Run only if Stage 1 found `lipc-get-prop`, while the Kindle is already awake. Do not substitute another publisher/property.

**PURPOSE:** Verify that the documented `state` read interface exists and record its awake-state terminology.

**EXPECTED INFORMATION:** A state string or an unavailable/permission error.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** The command uses the getter utility, supplies no value, and targets a property labeled readable in SRC-003. SRC-004 independently uses this `state` getter. No `lipc-set-prop` operation is present.

**PORTABILITY / AVAILABILITY RISK:** High for target availability/permission. Terms such as `screenSaver`, `suspend`, or `ready` are service labels and cannot prove kernel residency.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Validates an available observation channel for later UNK-002/UNK-003 work; resolves neither.

### Observation S2-10 — Narrow powerd status getter

**COMMAND OR OBSERVATION:**

```sh
lipc-get-prop com.lab126.powerd status
```

Run only if Stage 1 found `lipc-get-prop`, while the Kindle is already awake. Do not substitute another publisher/property.

**PURPOSE:** Verify that the documented `status` read interface exists and learn which awake-state power fields it already exposes.

**EXPECTED INFORMATION:** A status string or an unavailable/permission error.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** The command uses the getter utility, supplies no value, and targets a property labeled readable in SRC-003. No `lipc-set-prop` operation is present.

**PORTABILITY / AVAILABILITY RISK:** High for target availability/permission and medium for privacy. Redact battery details if desired. Labels and countdowns remain powerd terminology and cannot prove kernel residency.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Identifies fields that may be correlated in later UNK-002/UNK-003 work; resolves neither.

### Observation S2-11 — Narrow window-manager getter

**COMMAND OR OBSERVATION:**

```sh
lipc-get-prop com.lab126.winmgr isScreenSaverLayerWindowActive
```

Run only if Stage 1 found `lipc-get-prop`, while the Kindle is already awake. Do not substitute a read/write or write-only property.

**PURPOSE:** Verify whether a documented screen-saver-layer status getter exists on the target firmware.

**EXPECTED INFORMATION:** A value or an unavailable/permission error.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** The command uses the getter utility with no value and targets a property labeled read-only in SRC-003.

**PORTABILITY / AVAILABILITY RISK:** High; SRC-003 is incomplete and not Scribe-scoped. The property name and value do not establish who writes the framebuffer or refreshes the panel.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Determines whether a candidate observable exists for later UNK-001/UNK-003 tracing.

### Observation S2-12 — Available log surfaces and existing kernel PM messages

**COMMAND OR OBSERVATION:**

```sh
ls -la /var/log 2>&1
dmesg 2>&1 | grep -Ei 'PM:|suspend|resume|wakeup|wake up|woke|sleep'
```

Run the second line only if Stage 1 found `dmesg`.

**PURPOSE:** Inventory log filenames and determine whether the existing kernel ring buffer already contains narrowly filtered suspend/resume/wake messages.

**EXPECTED INFORMATION:** Log-surface names plus zero or more matching kernel messages from prior ordinary device activity.

**SAFETY CLASSIFICATION:** READ_ONLY.

**WHY IT IS BELIEVED READ-ONLY:** `ls` reads directory metadata. `dmesg` without `-c` or a console-level option reads the ring buffer; `grep` filters captured text. No log configuration, trace enable, clear, or rotation command is used.

**PORTABILITY / AVAILABILITY RISK:** High; kernel access may be denied, terminology may not match, history may be overwritten, and matches can be unrelated. Inspect locally and redact identifiers before retaining output. Do not run `showlog`, `logread`, or read whole application logs in this first probe.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Determines whether a candidate kernel oracle exists for UNK-002 and what may be available for later UNK-003 correlation. Pre-existing lines do not prove a controlled trial entered low power.

## Commands that remain UNCERTAIN and must not run in Phase 2A

Each entry is a proposed future observation that failed the first-interaction gate.

### Uncertain U-1 — Broad LIPC probe

**COMMAND OR OBSERVATION:** `lipc-probe -a -v`

**PURPOSE:** Would enumerate publishers, properties, and events broadly.

**EXPECTED INFORMATION:** A large LIPC inventory.

**SAFETY CLASSIFICATION:** UNCERTAIN.

**WHY IT IS BELIEVED READ-ONLY:** Its name and SRC-003 usage suggest discovery, but the pinned documentation does not establish which calls it makes against every publisher/property.

**PORTABILITY / AVAILABILITY RISK:** High; target implementation and per-property effects are unreviewed, output is broad, and it may expose unrelated/private state.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Could help UNK-001/UNK-003/UNK-006, but is unnecessary until its implementation or target help/behavior is safely reviewed.

### Uncertain U-2 — Event subscription

**COMMAND OR OBSERVATION:** `lipc-wait-event -m -s 0 com.lab126.powerd goingToScreenSaver,outOfScreenSaver,exitingScreenSaver,readyToSuspend,wakeupFromSuspend`

**PURPOSE:** Would observe the Phase 1 event set during a later controlled transition.

**EXPECTED INFORMATION:** Event names, payloads, and ordering.

**SAFETY CLASSIFICATION:** UNCERTAIN.

**WHY IT IS BELIEVED READ-ONLY:** FACT-001 shows KOReader using a subscription-only form, but target availability, exact argument portability, suspend behavior, and observer effect have not been established on this Scribe.

**PORTABILITY / AVAILABILITY RISK:** High. It is long-lived, can affect the process/network environment, and belongs to Phase 2B rather than awake-state discovery.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Later UNK-003 and wake-path parts of UNK-006; it cannot independently prove UNK-002.

### Uncertain U-3 — Whole-bus monitor

**COMMAND OR OBSERVATION:** `dbus-monitor --system`

**PURPOSE:** Would observe system-bus signals and possibly other traffic.

**EXPECTED INFORMATION:** A broad live bus stream.

**SAFETY CLASSIFICATION:** UNCERTAIN.

**WHY IT IS BELIEVED READ-ONLY:** Monitoring is nominally observational, but the target policy, runtime footprint, volume, and sensitive-data exposure are not bounded.

**PORTABILITY / AVAILABILITY RISK:** High; it may omit method calls, capture unrelated personal/application events, and remain active during suspend. SRC-003 explicitly notes that expanding eavesdrop permissions would require a dangerous configuration change, which is prohibited.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Could contribute to UNK-001/UNK-003, but the broad form is not minimum or privacy-safe.

### Uncertain U-4 — Framebuffer ioctl metadata utility

**COMMAND OR OBSERVATION:** `fbset -i`

**PURPOSE:** Would request fixed/variable framebuffer information through framebuffer ioctls.

**EXPECTED INFORMATION:** Geometry, timings, pixel layout, and driver metadata.

**SAFETY CLASSIFICATION:** UNCERTAIN.

**WHY IT IS BELIEVED READ-ONLY:** `-i` is commonly informational, but neither the installed utility nor the target Scribe driver ioctl path has been validated in the pinned evidence.

**PORTABILITY / AVAILABILITY RISK:** High; utility variants differ, Scribe uses a modern MTK path in community projects, and sysfs metadata is sufficient for the first interaction.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Could narrow UNK-004, but cannot compare framebuffer bytes with the physical panel.

### Uncertain U-5 — Vendor/system log helper execution

**COMMAND OR OBSERVATION:** `showlog` or `logread`

**PURPOSE:** Would retrieve broader stock/application logging.

**EXPECTED INFORMATION:** Potential powerd, blanket, window-manager, or kernel messages.

**SAFETY CLASSIFICATION:** UNCERTAIN.

**WHY IT IS BELIEVED READ-ONLY:** The names suggest retrieval, but the target scripts/binaries have not been inspected for collection, rotation, cursor, or other side effects.

**PORTABILITY / AVAILABILITY RISK:** High; output may be large and contain account, network, document, or notebook-related metadata. Stage 1 checks only whether the tools exist.

**WHAT PHASE-1 UNKNOWN IT HELPS ANSWER:** Could later inform UNK-001/UNK-003/UNK-006 after implementation and privacy review.

## Explicitly excluded operations

The probe must not execute `lipc-set-prop`; write or `echo` into `/sys` or `/proc`; read or write `/dev/fb0` pixel data; issue refresh/draw operations through eips, FBInk, or another tool; start, stop, restart, reload, or signal a service/process; invoke suspend; inject a power button, key, touch, pen, cover, or other input event; install a package; change startup/init; remount; modify the root filesystem; clear/enable/reconfigure logs; or enable ftrace/perf/audit facilities. Those operations are outside Phase 2A. Many are **STATE_CHANGING**; framebuffer pixel reads are excluded additionally for privacy even though a read might not change panel state.

## Candidate suspend oracles for later phases

This section evaluates possibilities only; it authorizes none of them. “Direct” means direct evidence of either a kernel suspend transition/residency counter or electrical power, not proof of the complete StayInk invariant. No current evidence makes `readyToSuspend`, screensaver entry, `wakeupFromSuspend`, or a powerd log sufficient proof of low-power residency.

| Candidate oracle | Direct or indirect | Survives/records across suspend? | Depends on powerd terminology? | External hardware? | Confidence from current evidence | Limits and later use |
| --- | --- | --- | --- | --- | --- | --- |
| Existing kernel PM entry/exit messages in the kernel ring buffer | Direct for a kernel PM transition; indirect for depth/current | Usually retained in RAM across suspend, but may be absent/overwritten and does not survive reboot reliably | No | No | Medium as a concept; target availability **UNKNOWN** | Correlate entry and resume lines with a controlled trial. A successful callback sequence does not quantify low-power residency. |
| `/sys/power/suspend_stats` or debugfs suspend success/failure counters | Direct for the kernel's recorded transaction result; indirect for depth/duration/current | Expected to remain in kernel memory across suspend; reboot behavior and Scribe availability **UNKNOWN** | No | No | Medium-low | Compare before/after only after field semantics are identified. A success increment is stronger than powerd state but still not electrical evidence. |
| Kernel `power:suspend_resume` tracepoint | Direct for kernel transition phases; indirect for actual current | A configured in-RAM trace may span suspend; target behavior **UNKNOWN** | No | No | Medium | Enabling tracing is **STATE_CHANGING** and may perturb timing; it requires a separate reviewed experiment and rollback. |
| Difference between `CLOCK_BOOTTIME` and `CLOCK_MONOTONIC` deltas | Indirect kernel time-accounting evidence for time spent suspended | Computed after resume; no in-suspend logger is needed | No | No | Medium-low; no target validation | Requires a small reviewed observer or existing safe tool. It estimates suspended time but not suspend depth/current. |
| SoC/platform residency counters, if exposed and documented | Potentially direct for the documented hardware power domain; otherwise indirect | Counter-dependent; may reset on resume or reboot | No | No | Low / **UNKNOWN** | No Scribe-specific counter or semantics is currently established. Do not interpret similarly named counters from another platform. |
| External current/energy logging during a stock control and later candidate trial | Direct electrical evidence of low-power behavior | Yes, if recorded by an independent logger | No | Yes | High in principle; target protocol **UNKNOWN** | Best independent complement to kernel evidence. USB attachment/charging can alter behavior; meter placement, baseline, sample rate, and safety require separate design. No battery disassembly is authorized. |
| powerd state/status and `readyToSuspend` / `wakeupFromSuspend` events | Indirect service-level evidence | Only if an observer/output channel records them; listener survival and buffering are **UNKNOWN** | Yes | No | High that selected apps observe labels; low as proof of low power | Useful for ordering, never sufficient for residency. |
| Loss of SSH/network response | Indirect | An external host can timestamp loss/recovery | No, but depends on networking | External host only | Very low | Network services can stop while CPUs remain active, and an SSH session itself perturbs the system. Not an oracle. |
| Battery percentage or temperature change over time | Indirect | A later read can be compared with baseline | Possibly, if read through powerd | No | Very low | Resolution and thermal/workload confounders make this unsuitable as primary evidence. |

**INFERENCE:** A later genuine-suspend claim should require at least one kernel-level transition/residency observation correlated with independent external power measurement against a stock control. This is supported by the complementary limits above but remains unverified on the target Scribe.

## Output handling and redaction

Capture a plain-text transcript on the development machine containing:

1. the manual UI record;
2. each exact command as entered;
3. stdout, stderr, and absent/permission errors;
4. Stage 1 and Stage 2 timestamps;
5. a note that the device stayed awake and no power/cover/input transition was attempted;
6. a redaction log stating which fields were removed.

Before retaining or sharing the transcript, redact full serial numbers, MAC/IP addresses, Wi-Fi SSIDs, account/customer identifiers, hostnames if personalized, document/notebook names or contents, process arguments containing personal paths, and unrelated application-log text. Do not copy framebuffer pixels or entire logs. Preserve unrecognized platform codenames, kernel/build strings, process executable names, service names, error messages, sysfs paths, and power/display metadata because those are the evidence needed for the next design step.

No output file should be created on the Kindle during this probe. If local terminal capture is unavailable, stop and redesign output transport rather than redirecting into `/mnt/us` without separate approval.

## Decision gate

### 1. Smallest useful first interaction

First complete Observation UI-1 without connecting. Then, in one already-awake shell session, run only Stage 1 (S1-1 through S1-8), capture output externally, disconnect, and review/redact it. This establishes target identity, firmware/kernel, privilege boundaries, mounts, and tool availability before any IPC getter or broader system inventory.

### 2. Exact commands safe enough to run

The commands marked **READ_ONLY** in Stage 1 and Stage 2 are safe enough under this design, subject to their stated fixed paths, exact arguments, awake-state use, and local output capture. Do not improvise alternatives. In particular, the only approved LIPC calls are:

```sh
lipc-get-prop com.lab126.powerd state
lipc-get-prop com.lab126.powerd status
lipc-get-prop com.lab126.winmgr isScreenSaverLayerWindowActive
```

### 3. Commands that remain uncertain

`lipc-probe -a -v`, the specified `lipc-wait-event` subscription, `dbus-monitor --system`, `fbset -i`, and execution of `showlog`/`logread` remain **UNCERTAIN** and must not run in Phase 2A.

### 4. Information to record manually before connecting

Record the exact UI wording for model/generation and firmware/build; physical model number if externally visible; date/time; battery/charging and Wi-Fi state; orientation; passcode, ads/Special Offers, and cover presence; and normal-UI-visible jailbreak/launcher/package-manager versions. Close personal content and use a non-sensitive stock Settings page. Do not record full device/account/network identifiers.

### 5. Output to copy back

Copy the redacted development-machine transcript described above: UI baseline, exact commands, all stdout/stderr/errors, timestamps, and redaction notes. Do not copy framebuffer contents, full application logs, or unrelated personal data.

### 6. Phase 1 unknowns that remain

All central behavior unknowns remain after Phase 2A:

- UNK-001: process and service inventories provide candidates but do not identify the stock sleep-screen renderer.
- UNK-002: existing kernel logs directly demonstrate `mem` suspend/resume transactions, but electrical low-power residency and the lowest hardware depth remain unknown.
- UNK-003: no transition occurs, so replacement timing relative to powerd events remains unknown.
- UNK-004: no suspend/resume or framebuffer/panel comparison occurs, so persistence remains unknown.
- UNK-005: no bypass/intervention is attempted, so genuine-suspend compatibility remains unknown.
- UNK-006: wake sources may be inventoried, but no wake path or UI restoration is tested.
- UNK-007: current privileges/mounts narrow the environment, but no safe future lifecycle is established.
- UNK-008: one firmware 5.19.5 target row is established, but its generation and portability to other models, firmware, and UI variants remain unknown.

### 7. Executable probe decision

No `scripts/device-probe.sh` was created. Manual execution confirmed the approved command behavior on this target, but an executable remains unjustified because output requires human privacy review, several broader tools remain unreviewed, errors must not trigger automatic fallback or escalation, and a remote session has observer effects.

## Completion record

Phase 2A is closed. Stage 1 and Stage 2 were completed, and the `ps w` limitation was resolved with the six-service targeted PID-to-procfs observation in S2-8A. The resulting process evidence is recorded as FACT-013; the existing kernel suspend evidence is recorded as FACT-014. No additional Phase 2A device commands are approved or needed. Phase 2B transition tracing requires a separate design and safety review.
