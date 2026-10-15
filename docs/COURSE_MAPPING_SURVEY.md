# Course Mapping / Live Survey Guide — v0.66.9.0-RC3.2

RC3.2 carries forward the RC3.1 survey-quality revision of the live course-mapping toolchain and adds parameterized survey execution/evidence closure. It deliberately lands before lengthy Stage-1 runs so those runs retain the evidence needed for course geometry, player trajectory, traffic/obstacle discovery and road-surface research.

## Live survey control

The mature native course follower can be configured, started, stopped and reset from Workbench, Research API and `.chqscript` without restarting the emulator. `course.survey.start` can hold the timer, suppress only the already-proven collision-response shove while preserving detection, clear/start the shared recorder and enable the follower; `course.survey.stop` restores the prior timer/collision/follower state.

The final RC3.0 fixes are mandatory and carried forward: `Set-CourseFollowConfig($config)` / `$config.ContainsKey($key)`, Track View routing to `Course Mapping / Live Survey`, and the Ctrl+P SDL pause fallback in addition to Pause/Break.

## Evidence model

The mapper separates authority from convenience:

- **Road geometry:** left edge, centre, right edge, page/selector, record, curvature, course position and authoritative road-state/lateral values.
- **Player driven line:** authoritative lateral position/error plus a separate projected map trajectory. The SVG line is a visual projection; raw lateral values remain authoritative.
- **Confirmed Stage-1 target:** object record `0x10A080-0x10A0BF` is emitted as `CONFIRMED_TARGET_CAR` only when the active checkpoint context is explicitly Stage 1; otherwise it remains `UNCLASSIFIED_TARGET_RECORD`. The scope/basis is retained in the export.
- **Raw visible sprites:** every visible row remains `UNCLASSIFIED_DYNAMIC_OBJECT` with slot/map/palette/bounds/priority/pixel evidence.
- **Object candidates:** RC3.1 conservatively turns each road-relevant visible sprite observation into an `UNCLASSIFIED_OBJECT_CANDIDATE` and carries candidate IDs across adjacent samples only when map + palette match and screen position is plausibly close. It deliberately does not merge touching sprite rectangles because repeated perspective scenery can form dense chains. Candidate tracks are not yet proven AI cars, obstacles or world-space objects.
- **Road/material candidates:** `surfaceStableSignature` is now the confirmed record `+4` `roadProcessingSelector` candidate key. `courseRecordSignature` separately preserves all eight bytes. Proven palette (+5), environment (+6), and feature (+7) metadata no longer fragment material candidates.
- **Raw renderer evidence:** full representative TC0150ROD scanline/control state remains in each sample as `roadRenderSignature`, plus a conservative masked `surfaceRenderCandidateSignature`. These values are evidence, not material labels.

RC3.1 specifically avoids the RC3.0 failure mode where dynamic perspective/scroll words made three samples of one opening road section look like three materials.

## Track Recorder v3 status

`api track.record.status` reports, among other normal recorder fields:

- `sampleCount`
- `surfaceCandidateCount`
- `surfaceSegmentCount`
- `rawSurfaceRenderVariantCount`
- `rawSpriteRowCount`
- `objectCandidateRowCount`
- `objectCandidateTrackCount`
- `persistentObjectCandidateTrackCount` (tracks observed in at least two samples)

This makes data quality visible before committing to a long survey.

## Mapping export v2

`api track.map.export name=NAME` writes under `evidence/track-maps/NAME/`:

- `track-map.json` — reconstructable master dataset, including raw evidence and candidate layers.
- `track-map-summary.json` — compact counts for samples, surface candidates/segments, raw renderer variants, raw sprite rows and candidate tracks.
- `track-samples.csv` — road/course/player/follower/surface telemetry.
- `car-trajectory.csv` — player line, speed, steering, road state and follower outputs.
- `track-entities.csv` — confirmed target rows plus the raw unclassified visible-sprite census.
- `track-object-candidates.csv` — conservative temporally tracked screen-space sprite candidates with stable research IDs where map/palette/position support continuity.
- `track-surfaces.csv` — +4 road-processing/material candidates with renderer-variant counts/examples.
- `track-surface-segments.csv` — contiguous runs of each +4 road-processing/material candidate.
- `track-map.svg` — road edges/centreline plus player trajectory visualisation.
- `README.txt` — dataset semantics and caveats.

## Recommended scripts

1. `regression/regression-v06690-rc31-course-mapping-quality.chqscript` — first focused proof on a new Windows build.
2. `track/surveys/stage1-course-mapping-shakedown-600.chqscript` — short Stage-1 quality run; inspect this before anything longer.
3. `track/surveys/stage1-course-mapping-3600.chqscript` — main centre-follow Stage-1 survey.
4. `track/surveys/stage1-course-mapping-negative-bias.chqscript` and `...positive-bias...` — controlled structural branch coverage. Negative/positive are not semantic left/right labels.
5. `track/surveys/current-state-course-mapping-1200.chqscript` — generic current-checkpoint survey.
6. `track/surveys/current-state-material-object-survey-1200.chqscript` — later-stage/current-state reconnaissance focused on surface and object evidence with regular screenshots.

## Interpreting traffic and obstacles

Do not promote a candidate merely because it looks like a car or obstruction in one screenshot. Use temporal consistency, sprite map/palette membership, collision behaviour, course position, screenshots and—where discovered—authoritative game-object provenance. Retain the raw sprite rows even after a candidate gains a higher-level identity so later conclusions remain auditable.

The confirmed Stage-1 target already has authoritative world/course-related state. Generic candidates currently have screen-space tracks only; deriving authoritative course/world positions for ordinary AI traffic/obstacles remains future reverse-engineering work.

## Interpreting road material

Likewise, `surfaceStableSignature` is a +4 road-processing segmentation key, not an asphalt/dirt label. `courseRecordSignature` is the full record identity. A material should only be promoted after repeated visual/state evidence shows a stable relationship between the +4 candidate, renderer state and appearance/behaviour. +5 palette, +6 environment and +7 feature metadata are explicitly excluded from the material key. Raw renderer variants are retained so a future material model can be rebuilt without rerunning the course.

## Object candidate evidence contract

RC3.1 keeps raw sprite census and object-candidate evidence separate. Under the current conservative policy, each visible sprite observation whose visible bottom reaches the road-relevant region (>=64) becomes one `UNCLASSIFIED_OBJECT_CANDIDATE`; nearby sprites are not merged merely because their rectangles overlap.

Candidate rows must preserve source slot/map/palette, member count, visible bounds, centre position, dimensions and visible-pixel count in addition to temporary tracking metadata. The recorder exposes `eligibleObjectCandidateSourceRowCount`, `objectCandidateRowCount` and `objectCandidateCoverage`; the current policy expects coverage `1.0`. `track.map.export` treats a count mismatch or lost candidate evidence as an error rather than silently writing a misleading map dataset.

Temporal candidate IDs remain heuristic screen-space associations, not semantic AI/obstacle identities. Raw sprite rows remain authoritative forensic evidence and should be retained even after candidate tracking improves.

## Deterministic survey sampling (RC3.1 cumulative fix)

Scripted course surveys are authoritative only when recorder sampling is deterministic. RC3.1 therefore suppresses a second `track.record.sample` request at the same emulator frame and reports `duplicateFrameSuppressed=true` instead of advancing the projected map geometry twice. The Track View also avoids its convenience auto-sample while a Script Console run is active. This keeps a 600-frame 15-frame-interval survey at exactly 41 unique samples and prevents UI navigation from contaminating scripted evidence.

Production visible-sprite rows are ordered dictionaries; the shared property accessor handles `IDictionary` keys case-insensitively so candidate coverage and map/palette/bounds evidence use the same representation encountered during live surveys.


### RC3.1 long-survey signed-16 correction

The first 3600-frame centre survey reached frame 3399 with 89 unique samples, all ON_ROAD, before a Workbench-only signed conversion failed on raw `0xFEF7` (65271), which semantically represents -265. Track-state road/lateral 16-bit deltas must use the explicit `Signed16` two's-complement helper rather than direct `[int16]` range casts. The corrected candidate includes a PowerShell runtime regression and static guard for this exact class of failure.
