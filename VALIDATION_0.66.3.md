# v0.66.3 Validation

Run `research/scripts/regression/regression-v0663-contracts.chqscript`. Verify `schema.all`, `script.schema`, and `sprite.map.inspect slot=45`.

Manual prompt test:
```text
api ui.prompt message=PRESS_ENTER_TO_CONTINUE pause=true top=true timeout=300000
```
Verify the message appears over SDL, Enter continues and Escape cancels.

After any non-validation Script Console run verify `<session>/script-runs/<run-id>/` contains `script.chqscript`, `console-output.txt`, `run-metadata.json`, `bundle-manifest.json`, `artifacts/`, and `<run-id>.zip`.
