# Kindle Display Path

## Framebuffer memory is not the visible panel

Classification: FACT — FACT-005

FBInk demonstrates two separate operations in its supported environments:

1. map and change framebuffer memory; and
2. ask the E-Ink driver to update a panel region.

Its `no_refresh` option permits memory changes without a panel update, while `fbink_refresh` issues a refresh without painting new content. A future experiment must observe both the bytes in the framebuffer and visible panel output; either alone can be stale.

## Scribe uses a modern MTK path in both references

Classification: FACT about the projects — FACT-006

FBInk flags first-generation Scribe device identifiers as MTK and dispatches updates through `refresh_kindle_mtk`, which selects a waveform and full/partial update mode before `MXCFB_SEND_UPDATE_MTK`. KOReader also flags Scribe as MTK, creates its `framebuffer_mxcfb` screen, and toggles an MTK fast-mode control.

FBInk implements multiple waveforms and update modes. That establishes that refresh policy matters; it does not establish which mode the stock sleep renderer uses or which mode StayInk should use. No waveform is selected for StayInk in this phase.

## Persistence through power loss is physically plausible, not Scribe-confirmed

Classification: FACT for general E Ink technology — FACT-010

E Ink describes its display medium as bistable: a static image does not require continuous power to remain visible. Therefore a genuine system suspend need not inherently blank an E-Ink panel.

Classification: UNKNOWN for Kindle Scribe

We have not demonstrated that Scribe leaves the controller and panel unchanged across suspend, that its resume path does not reinitialize or redraw the panel, or that framebuffer memory after resume still corresponds to the visible image. FACT-011 is direct evidence that long-sleep wake behavior can differ from short sleep while KOReader is active.

## Implications for candidate experiments

- Capturing framebuffer bytes does not prove those bytes are currently visible.
- Restoring bytes without a refresh may not change the panel.
- Refreshing without correct MTK parameters can fail or destabilize the display driver; FBInk contains explicit region-safety checks and an MTK animation crash warning.
- A direct framebuffer experiment must use a non-sensitive test screen. Capturing a notebook page would collect user content even if it did not modify notebook data.
- A successful visual result must still be paired with independent evidence that real suspend and normal wake occurred.

The unresolved display questions are tracked as UNK-001, UNK-003, and UNK-004.
