# RC2.3 TC0100SCN Y-scroll A/B evidence

Source upload: `tc0100-y-sign-ab.zip` (2026-09-30).

The four bounded runs use the same `stage1-gameplay-2064.chqstate` and capture frame 2065. Only BG0/BG1 presentation Y offsets differ:

- current: BG0 `-4`, BG1 `-20`
- BG0 sign proxy: BG0 `-64`, BG1 `-20`
- BG1 sign proxy: BG0 `-4`, BG1 `-64`
- both sign proxy: BG0 `-64`, BG1 `-64`

The both-sign proxy moves both TC0100SCN building planes from the lower road/foreground to the skyline. With `CTRL3=CTRL4=0x01E0`, the RC2.3 old source-Y formula plus a `-64` output-space proxy is mathematically identical to the RC2.4 corrected formula with neutral BG presentation offsets. Therefore `expected-frame-2065.png` is the Windows/SDL visual target for RC2.4 default geometry.

Nested evidence ZIP SHA-256:

- `tc0100-y-bg0-sign-proxy_20260930-001033.zip`: `ece6d35f2b16b13eba3cf89f7850936617fceb61be6a32391da0b027b4cb0607`
- `tc0100-y-bg1-sign-proxy_20260930-001034.zip`: `5e3aa3e8006ccec3f06a6f482bfa3c7ec381e42f0e166f5b178e4a9295e8cc87`
- `tc0100-y-both-sign-proxy_20260930-001036.zip`: `212924ee7cf4948b7110a97d8856b8e2972b30b852f920f58fcc6f5c507a9984`
- `tc0100-y-current_20260930-001032.zip`: `6fbb052d01a83826227fb45579237174cf3f0d20d9822e5f895d0e2513fa3675`
