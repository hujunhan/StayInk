# StayInk

Keep what's on your Kindle's E-Ink screen visible while the device sleeps, instead of replacing it with the stock screensaver.

StayInk is an unofficial, open-source utility for **jailbroken** Kindle e-readers, developed first for the **Kindle Scribe**. It is not affiliated with Amazon.

## Goal

```text
current page or note  →  sleep  →  screen stays as-is during genuine low-power suspend  →  wake  →  normal UI
```

The hard rule: the device must still really suspend. Keeping the screen by blocking sleep doesn't count.

## Status: research, nothing to install yet

The project is evidence-first. Each claim about Kindle internals is labeled **FACT**, **INFERENCE** or **UNKNOWN**. Device work moves from read-only probing, to reversible runtime experiments, to persistent changes, in that order.

| Phase | State |
|---|---|
| 1. Platform research (power, display, packaging, Scribe) | done ([docs/research](docs/research/README.md)) |
| 2. Read-only device probe and power traces on a Scribe | done |
| 3. Locating where the screensaver is drawn and presented | done |
| 4A. Bounded window-title (`WM_NAME`) experiment | designed, not run |
| 4B. Experimental guard with a watchdog ([tools/hide-guard](tools/hide-guard/README.md)) | host-tested; the default build cannot write |

## Repository

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md): the intended behavior
- [`docs/SAFETY.md`](docs/SAFETY.md): the safety model that applies before any change is made on a device
- [`docs/research/`](docs/research/README.md): facts, hypotheses, unknowns and experiment records
- [`tools/hide-guard/`](tools/hide-guard/README.md): the C guard, with host tests and an import audit
- [`AGENTS.md`](AGENTS.md): rules for AI coding agents working in this repo

## Safety

Jailbreaking and modifying a Kindle can brick it or void its warranty. Nothing here is a release. Don't run experimental builds on a device you can't recover.

## License

[Apache-2.0](LICENSE). No GPL or AGPL code is copied in; other projects are used only as references.
