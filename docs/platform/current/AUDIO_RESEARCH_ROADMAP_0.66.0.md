# Audio Research Roadmap — v0.66.0

Audio is intentionally a first-class research namespace from v0.66.0 onward, while runtime mutation remains gated until Windows/SDL validation.

## Phase A — observation
- authoritative sound CPU / command-bus event trace
- mixed PCM capture with frame and sample-clock timestamps
- channel/source inventory
- waveform viewer with zoom, scrub, loop selection and event markers
- deterministic A/B audio capture aligned to checkpoints

## Phase B — extraction/export
- identify sample ROM / decoded voice sources
- export user-ROM-derived decoded one-shot samples to WAV/raw PCM with provenance metadata
- identify music/sequencing representation and export research metadata/events
- never bundle copyrighted ROM/audio data in releases

## Phase C — controlled replacement/injection
- offline replacement preview first
- source-isolated/muted live replacement
- runtime sample replacement with automatic restore and volume clamp
- later music/stream injection if synchronization is proven safe
- every injection recorded in experiment evidence with source hash and exact timing

## Workbench UI direction
- Audio Lab tab
- waveform/spectrogram view
- channel mute/solo
- event timeline correlated with gameplay/sprite/track state
- download/export buttons for user-derived research captures
- A/B blink equivalent for audio: original/modified toggle and short alternating audition regions

## Script/API direction
Current foundation actions: `audio.capabilities`, `audio.channels`, `audio.events`, `audio.waveform.plan`, `audio.export.plan`, `audio.inject.plan`.

Future actions should follow the existing evidence/experiment model rather than creating a separate audio-only workflow.
