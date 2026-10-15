# Reviewed REA/static-analysis inputs

Files named `CASE.static.json` and `CASE.census.tsv` are operator-staged for `api rea.crosswalk name=CASE`. They are untrusted research inputs, not executable content. Use the Native Lab local import form to stage them safely. Crosswalk output goes to `evidence/rea-crosswalks/CASE.json` and the active script-run artifacts. `demo-only.*` is **synthetic** with a zero ROM SHA, and must never be used as evidence of a real Chase H.Q. function boundary or real static analysis. Do not bundle ROM binaries.
