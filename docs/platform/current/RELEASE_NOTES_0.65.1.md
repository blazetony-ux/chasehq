# ChaseHQ-Native v0.65.1 — Experimental Profile Plumbing

This is a deliberately small additive release over v0.65.0. It does **not** refactor or redirect Chase H.Q. emulation logic.

## Added
- Read-only `games/<id>/profile.json` descriptors.
- Authoritative Chase H.Q. profile.
- Metadata-only SCI cross-game probe profile.
- Taito Z hardware-boundary descriptor.
- `GET /api/v1/profiles` and `GET /api/v1/profile?game=<id>`.
- `api profiles.list`, `api profile.get`, and `getprofiles()`.
- Profile metadata regression coverage.

## Safety boundary
SCI cannot be activated or loaded by this release. The profile layer exists solely to quantify future cross-game compatibility without jeopardising Chase H.Q. progress.
