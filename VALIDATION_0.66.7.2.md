# v0.66.7.2 Validation

Purpose: prove the two documentation-integration fixes discovered after the successful v0.66.7.1 regression.

Required Windows/SDL proof:
1. `api docs.handover` must render UTF-8 punctuation correctly (no mojibake such as `â€”` or `Ã—`).
2. Run `Regression - v0.66.7.2 Workbench Knowledge / SDL / Sprite Controls`.
3. The resulting run bundle must contain `artifacts/documentation-exports/regression-v06672-docs.html`.
4. Layered snapshot reconstruction must remain `verifiedExact=true` and `mismatchedPixels=0`, with individual sprite assets present.
5. SDL window status/control checks must still succeed.
