# Architecture

## Desired product behavior

```text
CURRENT DISPLAY
      |
      v
sleep requested
      |
      v
avoid unwanted display replacement
      |
      v
genuine suspend
      |
      v
wake
      |
      v
normal UI restored
```

StayInk must leave the currently displayed E-Ink contents visible during genuine low-power suspend, then preserve normal wake behavior.

## Technical mechanism

**UNKNOWN.** The mechanism must come from evidence recorded in `docs/research/` before implementation is designed.
