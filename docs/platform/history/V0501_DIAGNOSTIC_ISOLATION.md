# v0.50.1 Diagnostic isolation and autonomous probe batching

This patch fixes the case discovered by the `572/594` experiment where structured ownership evidence reported visible sprite pixels but screenshots were completely black. When a `--sprite-solo` selector is combined with a non-`none` diagnostic background, selected sprite pixels are now rendered in a raw-pen greyscale forensic palette. This is presentation-only: sprite selection, priority rejection, ownership and emulated machine state are unchanged. The normal game renderer continues using the native palette when diagnostic isolation is not active.

The project now also includes `tools/Run-SpriteProbeBatch.ps1`. It reads `tools/sprite_probe_batch.csv`, restores the same checkpoint for each candidate, runs the bounded evidence/screenshot experiment, creates one evidence ZIP per candidate, and finally creates a single `*-attachments.zip` containing those ZIPs. This directly addresses the manual sequence used for the 778-781, 268/269, 410 and 572/594 investigations.

Example:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\Run-SpriteProbeBatch.ps1 -Checkpoint .\ui_quick.chqstate -BatchName stage1-candidates
```

Batching is intentionally a thin orchestration layer over the existing command-line interface. Each individual run remains independently reproducible from its recorded argv/metadata.
