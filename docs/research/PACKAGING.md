# Modern Packaging and Runtime

## Supported KPM lifecycle

Classification: FACT — FACT-007, FACT-008

KPM's default package payload location is `/mnt/us/kmc/kpm/packages`, with its database and temporary data also under `/mnt/us/kmc/kpm`. The reviewed package interface has three optional shell hooks:

| Hook | Observed trigger |
| --- | --- |
| `install.sh` | after package extraction |
| `launch.sh` | explicit KPM/launcher launch |
| `uninstall.sh` | before KPM deletes the package |

Hooks execute with the unpacked package as their working directory. The example package installs and owns a scriptlet in `/mnt/us/documents`; its uninstall hook checks that the scriptlet is still its own before removing it.

## Root-filesystem avoidance

Classification: FACT about ecosystem guidance — FACT-009

KindleModding documentation says KPM hooks must not write to or remount rootfs and calls rootfs modification unsupported. Its recommended user-facing entry point is a scriptlet in `/mnt/us/documents` that delegates to `/var/local/kmc/bin/kpm launch <package>`.

KPM itself has platform components under `/var/local/kmc`, while its reviewed default keeps ordinary package payloads under `/mnt/us`. This distinction matters: using an existing jailbreak platform service is different from installing StayInk into the Kindle root filesystem.

## Background and device-lifecycle behavior

Classification: UNKNOWN

The reviewed KPM interface does not define boot, suspend, resume, or long-running-service hooks. Scriptlets provide a user launch surface; they are not evidence of an automatic background lifecycle. The KindleModding boot-process page describes Upstart on Paperwhite 6 only, so it cannot establish a Scribe service model.

No daemon or service architecture is proposed. Packaging is deferred until research establishes whether a future mechanism needs background presence at all and, if so, how it can start and stop without rootfs changes and with an explicit uninstall path.
