ChaseHQ-Native course mapping dataset
build=0.67.0.0-RC6.3.33
samples=3
track-map-summary.json: compact survey quality/count summary
track-samples.csv: authoritative sampled road/course state plus map projection
car-trajectory.csv: player driven line and autonomous-driver telemetry
track-entities.csv: confirmed target-car state plus raw unclassified live sprite census
track-object-candidates.csv: per-sprite observations with heuristic screen-space temporal track IDs over raw sprite evidence; NOT semantic AI/obstacle proof
track-surfaces.csv: +4 road-processing/material candidates with raw renderer-variant counts
track-surface-segments.csv: contiguous +4 road-processing candidate sections
track-map.json: reconstructable master dataset including raw evidence and candidate layers
track-map.svg: road edges/centreline + player trajectory visualisation
