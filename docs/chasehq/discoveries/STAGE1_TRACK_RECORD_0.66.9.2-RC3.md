# Stage-1 track-record semantics - v0.66.9.2-RC3

This document consolidates the causal/machine-code evidence gathered after v0.66.9.1 promotion, through research Runs 019-029. It records only the meanings established by those experiments.

## Native-ready record schema

| Offset | Name | Evidence status |
|---:|---|---|
| +0 | `curve` | confirmed horizontal curvature |
| +1 | `shapeTarget` | confirmed three-band renderer/shape target path; physical naming remains conservative |
| +2 | `featureType` | confirmed feature/type class and dispatch input |
| +3 | `geometryCode` | confirmed geometry control byte |
| +4 | `roadProcessingSelector` | causal road-processing/material-path selector |
| +5 | `paletteTransitionSelector` | CPU-A palette transition selector; not geometry/object selection |
| +6 | `environmentTransitionSelector` | nonzero selector triggers environment/scenery rebuild |
| +7 | `featureParameter` | polymorphic, interpreted according to feature/type path |

Legacy API names `flagsType`, `eventCode`, `visualSelector`, `featureTrigger`, and `controlFlags` are compatibility aliases only and are deprecated from this candidate.

## +4 road processing

Runs 003-018 established a causal path from record `+4` through CPU-B cached state (including `0x1021CA`) into downstream road processing and TC0150-visible output. Known values include `04`, `05`, `06`, and `07`; `06` produces the proven special/rough road-processing behaviour in the tested Stage-1 material branch. The semantic contract is deliberately `roadProcessingSelector`, not a universal asphalt/dirt enumeration.

## +5 palette transition

Run 019 separated the visible layer effects; Runs 020-021 decoded CPU-A consumption at `0x000EE6`, high-selector palette-bank rebuilds from ROM table `0x2EEE6`, and the `selector & 0x7F` four-target table at `0x2E952`. This byte belongs to palette/visual-transition processing, not road geometry.

## +6 environment transition

Runs 022-024 proved a direct CPU-A consumer at `0x601E`. Nonzero `+6` schedules callback `0x5F76`, which indexes the ROM word table at `0x2BCF4` and rebuilds TC0100SCN scenery. In Stage 1 the observed natural domain included `00`, `01`, and `04`; tested `01` maps environment mode 3 and tested `04` maps environment mode 1. Causal comparisons changed scenery/background while the raw road layer remained identical.

## +7 polymorphic feature parameter

Runs 025-029 show that `+7` does not have one universal bit meaning. Some `+2` classes consume it as a bitfield or low-bit parameter. The separate CPU-A helper at `0x9418` consumes it as signed/zero route-feature state for the proven `featureType=0xEB` family.

For that tested family only:

- `FF` takes the negative path via `0x53CA` and causally suppresses/clears the route indicator.
- `00` takes the zero path via `0x53B4`; in the tested Stage-1 state it produces route-arrow sprite map 764, palette 110.
- `02` takes the explicit positive `D2==2` path at `0x546E`; in the same state it reaches the common object path with object code `0x260019` and produces route-arrow variant map 766, palette 110.
- Other positive values are not assigned a global semantic meaning.

CPU-B `0x4B52`, observed while looking at `+7=02`, is a bulk-copy instruction and is not itself a semantic `+7` consumer.

## Signature contract

A complete course record and a road/material candidate are different identities. v0.66.9.2-RC3 therefore exposes:

- `courseRecordSignature`: all eight record bytes for exact record identity;
- `roadProcessingSignature`: record `+4` only;
- `roadMaterialCandidateSignature`: currently the proven `+4` road-processing selector, pending stronger material taxonomy;
- raw TC0150 `roadRenderSignature` / render candidate state separately.

`+5`, `+6`, and `+7` must not fragment road-material grouping. The legacy `courseSurfaceSignature` is retained only as a deprecated compatibility surface and now mirrors complete record identity rather than claiming material semantics.

## Checkpoint provenance

Canonical packaged checkpoints remain `stage1-driving-2352.chqstate` (ATTRACT MODE graphics reference) and `stage1-gameplay-2064.chqstate` (PLAYER GAMEPLAY), alongside the target/end-level canonical set. Runs 015-029 also used local gameplay-derived research checkpoints such as `stage1-page1-pre-candidate-a-3804.chqstate`, `stage1-page3-post-candidate-a-3864.chqstate`, and `stage1-page4-paved-record137-4584.chqstate`. Those local binaries are not fabricated or silently added to this package when the source bytes are unavailable; the run bundles remain their evidence provenance.
