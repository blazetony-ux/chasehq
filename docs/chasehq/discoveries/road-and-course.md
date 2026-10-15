# Discovery record - road, course and grade

**Status:** Stage-1 record semantics/topology and grade sign substantially behaviour-understood; directed graph/minimap still open.

The eight-byte Stage-1 course record is: `+0 curve`, `+1 shapeTarget/physical grade`, `+2 featureType`, `+3 geometryCode`, `+4 roadProcessingSelector`, `+5 paletteTransitionSelector`, `+6 environmentTransitionSelector`, `+7 featureParameter` (polymorphic by feature type).

## Physical grade semantics

Run018 causally resolved `+1`: positive signed values are **INCLINE/uphill**, negative values are **DECLINE/downhill**, zero is level. On a sustained controlled corridor, `-10` increased display/internal speed and course progress versus neutral; `+10` reduced them. The renderer and player-body presentation change consistently with that physical meaning.

## Road-side classifier

`0x10A048` encodes `01=ON_ROAD`, `03=OFF_ROAD_RIGHT`, `05=OFF_ROAD_LEFT`, `07=OFF_ROAD_EXTREME`. Run019 disassembly plus forced-position matrix resolved exact canonical-straight bands: with left `0x7470`, right `0x86F0`, outer limits are `0x7090`/`0x8AD0`; the side bands are one 0x100-wide region beyond the interior tolerance.

## Topology

Selector/fork lifecycle, branch choice and the Stage-1 0-9 transition family are already causally established. Treat the byte-semantics probe as closed. The next product is an authoritative **directed course graph/minimap** using the proven selector transitions, lateral fork rule, record addresses and live course/player position.

Do not conflate `+5/+6/+7` with road material identity. Keep full course record signature separate from road-processing/material candidates and raw TC0150 renderer state.
