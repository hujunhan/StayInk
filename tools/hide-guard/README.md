# Phase 4B experimental HIDE guard

This directory contains a purpose-built experimental guard for the bounded
Phase 4A `WM_NAME` transaction. It is not persistent StayInk behavior and does
not control Kindle power, display, services, input, or framebuffer state.

The default build is intentionally write-incapable. Its Linux object/binary
must not import `XChangeProperty`. An explicitly and separately named
write-enabled build exists for future review, but Phase 4B does not authorize
building, copying, or running that variant on the Kindle.

## Structure

- `include/hide_guard_core.h` and `src/hide_guard_core.c`: exact title,
  identity, transaction, deadline, and watchdog-state invariants.
- `src/hide_guard_linux.c`: Linux/Xlib identity adapter, dry-run modes,
  `CLOCK_BOOTTIME` timerfd watchdog, diagnostic/state records, and the single
  compile-gated `XChangeProperty` call site.
- `tests/hide_guard_core_test.c`: host tests using an injected fake writer.
- `audit.sh`: source/import audit for forbidden X11/display/power operations.

## Host tests

```sh
cmake -S tools/hide-guard -B /tmp/stayink-hide-guard-build \
  -DSTAYINK_BUILD_LINUX_GUARD=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/stayink-hide-guard-build
ctest --test-dir /tmp/stayink-hide-guard-build --output-on-failure
sh tools/hide-guard/audit.sh
```

## Linux build boundary

On Linux, the default build creates only `stayink-hide-guard`, compiled with
`STAYINK_ENABLE_WRITES=0`:

```sh
cmake -S tools/hide-guard -B build/hide-guard -DCMAKE_BUILD_TYPE=Release
cmake --build build/hide-guard
sh tools/hide-guard/audit.sh build/hide-guard/stayink-hide-guard nowrite
```

The separate `stayink-hide-guard-write-enabled` target appears only when
`-DSTAYINK_ENABLE_EXPERIMENTAL_WRITES=ON` is explicitly supplied. That option
is documented for source audit; it is not an execution authorization.

The Kindle requires a trusted ARM hard-float Linux cross-build compatible with
`/lib/ld-linux-armhf.so.3` and its Xlib ABI. Do not install a compiler or
development package on the Kindle. Record and verify the exact binary hash
before any target dry run.

## Modes

The write-disabled binary accepts only:

- `--dry-run --xid XID --pid PID`
- `--watchdog-dry-run SECONDS --xid XID --pid PID --log PATH`

It recognizes `--apply` and `--restore` only to refuse them and print that the
binary has no write capability. The dry-run watchdog creates an exclusive
diagnostic log, forks once, calls `setsid`, ignores `SIGHUP`, redirects its
standard descriptors to `/dev/null`, arms an absolute `CLOCK_BOOTTIME`
timerfd, sends an `ARMED` byte to the parent, and exits after its expiry-side
identity/property check.

The target procedures remain unexecuted and are specified in
`docs/research/GUARD_WATCHDOG_VALIDATION.md`.
