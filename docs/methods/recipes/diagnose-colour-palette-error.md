# Recipe — diagnose incorrect colours

Do not begin by changing RGB constants permanently.

1. Freeze/reproduce one frame containing an obviously suspect colour.
2. Identify the contributing layer/object and palette index.
3. Capture the raw palette word before decoding.
4. Compare palette bank selection and entry contents across known-good contexts if available.
5. Test candidate bit/channel layouts and bit-depth expansion methods as reversible display overrides.
6. Distinguish four fault classes: wrong source palette data; wrong bank/index selection; wrong raw-word RGB decoding; wrong final pixel/output conversion.
7. Use live entry/bank overrides only as experiments and restore authentic state after each test.
8. Preserve raw value, decoded value, screen result and decoder configuration in evidence.

A visually better palette is not proof of correct hardware decoding; the raw-data path must agree with the evidence.
