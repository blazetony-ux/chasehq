# Chase H.Q. TC0040IOC gameplay polling and intervention-path finding

## Question
Which IOC path does CPU-A poll during Stage-1 player gameplay, and did the v0.63.2 held XOR intervention reach that path?

## Starting state
Canonical `stage1-gameplay-2064.chqstate`.

## Method
CPU-A memory trace on `0x400000-0x400003`, first without intervention and then with held port-3 XOR `01`. Separately inspect `input ports` before/after setting the XOR.

## Observations
Every captured gameplay frame followed the same selected-port sequence. CPU-A writes selector values to `0x400003` and reads data from `0x400001`. The key input poll is around PCs `0x002BB6-0x002BD2`. Baseline selector `03` returns `0x3F`.

The v0.63.2 API/debug layer reported port 3 as `0x3E` with XOR `0x01`, but the live CPU trace continued to read `0x3F` after resume. Source inspection showed the scenario/CLI frame runner rewrote the same XOR storage every frame, erasing the held debugger value.

## Conclusion
High confidence: selector 03 is genuinely polled every gameplay frame. High confidence: the v0.63.2 negative held-bit sweep cannot classify those bits because its intervention did not survive to the CPU read.

## Rejected interpretation
“Port 3 is unused” is rejected. The trace proves it is read every frame.

## Remaining question
After v0.63.3 fixes intervention layering, which individual port-3 bits correspond to accelerator/turbo/other control semantics?
