# v0.50.0 First Object-Identification Experiment

This build targets the repeated sprite-map family 778/779/780/781 with palette 134 found in the existing dynamic evidence. The maps repeatedly form exact 2x2 blocks at several sizes and positions. This is a candidate assembly, not yet a named Chase H.Q. object.

Recommended Windows/SDL command (single line):

```powershell
.\out\build\x64-Debug\ChaseHQNative.exe ".\roms\chasehq" --load-checkpoint ".\ui_quick.chqstate" --sprite-evidence-every 5 --sprite-evidence-from 2388 --sprite-evidence-to 2508 --sprite-solo "map=778|779|780|781,palette=134" --sprite-quad-candidate "name=quad778,tl=778,tr=779,bl=780,br=781,palette=134" --diagnostic-background black --screenshot-every 10 --screenshot-from 2388 --screenshot-to 2508 --exit-at-frame 2509 --evidence-bundle --evidence-name "stage1-quad778"
```

Expected result: the checkpoint loads at frame 2352, the run terminates at frame 2509, only the candidate map family is presented over black, periodic screenshots support the structured evidence, exact 2x2 matches are written as `sprite_objects_frame_*.csv`, and the complete run is packed into a single evidence ZIP.
