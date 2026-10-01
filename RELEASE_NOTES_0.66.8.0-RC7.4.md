# ChaseHQ-Native v0.66.8.0-RC7.4

Focused Workbench scalability hotfix after reproducible Chrome OOM at Full Regression step 646 (`api script.runs`).

- `script.runs` is now bounded/paginated and summary-only by default.
- `/api/v1/script/runs` defaults to 50 records and accepts `limit`, `offset`, `status`, `version`, and `session`.
- Script action `api script.runs` defaults to 25 records and supports the same filters.
- Historical run discovery no longer embeds full console logs, source text, or complete artifact path arrays in every list response.
- Browser command display truncates individual outputs above 200,000 characters while the authoritative complete console remains on disk/in the evidence bundle.
- Full Regression discovery calls explicitly request bounded history pages.
- Added focused RC7.4 bounded-history regression.
