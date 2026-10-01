# Chase H.Q. Native v0.52.0 — Geometry + Sprite Ordering Experiment Platform

## Purpose
v0.52.0 consolidates the evidence gathered after v0.51.2 into a new experiment baseline. It is deliberately not a claim that every geometry or sprite-order hypothesis is final hardware truth.

## Changes
- Default presentation candidates updated to BG0 Y=-16 and BG1 Y=-20 after the extended 2160–3120 moving-scene validation.
- Added runtime/config sprite ordering experiment: `--sprite-tie-break lower-slot|higher-slot` / `sprite_tie_break=...`.
- The default remains `lower-slot` until higher-slot behaviour is validated beyond the player-car body/shadow case. The option changes sprite traversal for the experiment; PROM-vs-road/BG/text mixer logic is unchanged.
- Targeted sprite CSV now distinguishes native, visible-area and presentation-adjusted coordinates and records the sprite presentation offset.
- Added an auto-packaging PowerShell sprite XY sweep helper under `tools/`.
- Updated roadmap/evidence policy to make automatic final ZIP creation the normal standard for multi-run experiments.
- Both curated CHQSTATE v1 checkpoints remain bundled.

## Deliberately deferred
The following remain important but are not falsely claimed complete in this build: deterministic analog steering injection, authoritative ROAD STATE/SURFACE, uncropped full-hardware-raster PNG export, semantic gameplay-state logger/UI, and the accumulated debugger/reporting defect batch. They remain next-build/workstream targets.

## Fresh Windows build
Expected extracted root: `ChaseHQ-Native-v0.52.0`

From that root in PowerShell:

```powershell
Remove-Item -Recurse -Force .\out\build -ErrorAction SilentlyContinue; cmake --preset x64-Debug; cmake --build --preset x64-Debug
```

Executable:
`\.\out\build\x64-Debug\ChaseHQNative.exe`

Do not reuse an older version's build/cache directory.
