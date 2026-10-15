# RC4.6 release verification, promotion and research evidence

**Promotion date:** 2026-10-10  
**Release:** `0.67.0.0-RC4.6`  
**Archive SHA-256:** `9e76a107e19f3e28a54a8a8b3bdaed630511e02f1da3e3e7ae274c07d863580f`  
**Windows tool:** `chqdev 0.5.13`  
**Previous immutable promoted baseline:** `0.67.0.0-RC4.5` / `443ffe623a38de4813691c436e05d6f92d9e113a8abae0219824fcc77e9aae11`

## Qualification authority

The operator pasted the output of `Get-Content .\out\chqdev\qualification.json`; it names the same RC4.6 archive and its exact SHA-256 and reports all **11 stages PASS**: build, clean, doctor, focused, full, package, preflight, promotable, run-history, runtime, test. This is a Windows run by the operator, not a claim that portable validation alone qualifies the release.

Independent Workbench evidence bundles uploaded in the same conversation:

| Source | Run | Commands | Duration | Status | SHA-256 |
|---|---|---:|---:|---|---|
| `bundle(20261010-144017).zip` | 001 0.67.0.0-RC4.6 focused release regression | 13 | 6064 ms | PASS (integrity PASS) | `ee677e0485617c3b298c111f63adb2fa5551a64c9261f5eaa1a948b665b8c129` |
| `bundle(20261010-144623).zip` | 002 Full Regression Suite | 1153 | 358680 ms | PASS (integrity PASS) | `4de93d115fcf3246fffbcd4e32cd233fd09a52e224e26c948773fe3b2ad82b9d` |
| `bundle(20261010-152600).zip` | 009 Run 035 - Genuine Ghidra Static and Runtime Crosswalk | 19 | 1859 ms | PASS (integrity PASS) | `ca2e63d49e590d3a5536a74dacaf40b8702de867a5a0bb13f8d87e83d23a0886` |

## Independent Ghidra/REA workflow confirmation

Ghidra evidence archive `ghidra-rea-run032-20261010-161351-650.zip` SHA-256 `87c8b0d061058bc4d9750bee1d3e8ff4ef09c2075988c66d812bfa01d1bfaad3` contains headless logs, analysis manifest, findings and static export. The user also supplied parsed `chq-ghidra-findings.json`; it verifies eight instruction-site windows from a CPU-A ROM with SHA-256 `5cf1c9e786a39795ebd8e99c8e8be0d2398cc4f74221067cf2844945308d6c5a`, 68000 big-endian, image base `0x000000`. The successful job `ghidra-20261010-161350-586bf542` was headless/static, not emulation or native CPU replacement.

At health PC `0xA112`, Ghidra's **unreviewed** containing function candidate begins `0xA10C` and ends exclusive at `0xA12A`; it has an observed `RTS` at `0xA128`. The Run 035 damage report recorded 7 hits across 7 distinct executed PCs in that candidate. The function name/boundary is still `hypothesis`. The other six PCs without Ghidra function ownership remain instruction-site windows, not proven functions.

Run 035 generated three genuine census-linked reports (`ghidra-contact-5587`, `ghidra-damage-6333`, `ghidra-post-final-9548`) by explicit `rea.crosswalk` from Ghidra static findings and earlier verified checkpoint ROM execution TSVs. These prove local static/runtime crosswalk functionality, **not** an upstream `morluto/rea` agent analysing raw ROM.

## Outstanding known RC4.6 limitations

`ghidra.results` file path handling; initial missing staged census causing `ghidra.import` count 0 even on a PASS Ghidra job; Java/Jython worker preflight; Script Run Ghidra success/failure artifact aggregation; no `ghidra.wait`; REA/Ghidra view confined to narrow Native Lab column; live ROM census not yet in Workbench; optional Capstone bridge not implemented. See `docs/INVESTIGATION_LIMITATIONS.md` LIM-015 through LIM-019. Independent Go Full Regression structural-budget check remains outstanding despite passing Windows runtime Full Regression.

## Promotion disposition

**Promoted by explicit user request.** The exact Windows-qualified archive is frozen, without renaming, repackaging or new source bytes. This separate documentation/promotion supplement reflects newer qualification and research proof; it is **not** itself a rebuilt or requalified release. New REA top-level-tab work is future scope, not an RC4.6 shipped feature. Preserve qualified ZIP, archive SHA, `qualification-user-pasted.json`, focused/full Workbench evidence, Ghidra evidence and Run 035 bundle.
