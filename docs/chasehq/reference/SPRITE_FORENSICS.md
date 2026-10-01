# Sprite Forensics — v0.49.0

v0.49.0 starts using the v0.48 diagnostic foundation to investigate Chase H.Q. graphics rather than adding infrastructure in isolation.

## Authoritative composed sprite evidence

The frontend can now export per-frame CSV after the reference compositor has run:

    --sprite-evidence-every 1
    --sprite-evidence-from 900
    --sprite-evidence-to 1100
    --sprite-evidence-dir logs/sprite_evidence

Each record contains frame, logical sprite slot, map/object selector, palette, decoded priority, transformed screen origin, rendered dimensions/bounds, exact count of pixels that survived final sprite ownership, and the exact visible-pixel bounding rectangle derived from the compositor's ownership buffer.

This deliberately follows the project evidence rule: do not infer from screenshots what the renderer can report directly. Screenshots remain supporting evidence.

## Current interpretation boundary

`screen_*` and rendered bounds are the renderer's current decoded interpretation. `visible_*` is stronger evidence: it is measured from the final accepted logical sprite-owner buffer produced during composition. A visible bound of -1 means the sprite contributed no final pixels.

Raw sprite RAM, spritemap words, chunk destinations and alternate byte-order candidates remain available through the existing `--sprite-debug*` and `--sprite-export*` facilities. Those raw values should be retained alongside decoded evidence whenever a hardware interpretation is still uncertain.

## Next investigations

1. Correlate one visible sprite from raw RAM -> spritemap -> transformed coordinates -> final visible ownership.
2. Add presentation-only hide/solo controls without changing emulated state.
3. Add solid diagnostic backgrounds and exact ownership-derived masks.
4. Extend structured evidence with chunk/component identity and mixer/PROM decisions.
5. Progress toward selected-pixel provenance: what candidates reached a pixel and why the winner won.

The immediate target remains Chase H.Q. correctness, especially sprite composition and priority behaviour.

## v0.50.0: first candidate-object pass

The frame 2388-2688 evidence showed repeated map 778/779/780/781 entries using palette 134. They frequently form exact 2x2 arrangements with equal zoomed component dimensions: 778 at top-left, 779 top-right, 780 bottom-left and 781 bottom-right. Multiple such arrangements can coexist at different scales/positions, so a sprite RAM slot cannot be treated as object identity.

v0.50.0 adds two deliberately focused mechanisms:

- selector value sets such as `map=778|779|780|781,palette=134`, usable from the CLI for solo/hide experiments;
- `--sprite-quad-candidate`, which detects exact geometric 2x2 assemblies and emits `sprite_objects_frame_XXXXXXXX.csv` alongside normal authoritative per-sprite evidence.

A quad match is an observation about geometry, map identity, palette/priority and synchronized size. It is **not** automatically a semantic identification. Visual isolation and subsequent traces are required before naming the game object.

### Current experiment

```text
--sprite-solo "map=778|779|780|781,palette=134"
--sprite-quad-candidate "name=quad778,tl=778,tr=779,bl=780,br=781,palette=134"
--diagnostic-background black
```

Success criterion: determine what the isolated candidate family is, establish whether its 2x2 component relationship is stable enough to treat as a logical assembly, and then use that knowledge for a concrete graphics/compositor correction or a more narrowly targeted next hypothesis.
