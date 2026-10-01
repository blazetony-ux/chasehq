# Experimental game-profile layer — v0.65.1

## Purpose
The profile layer is a **read-only discovery/metadata boundary**, not a multi-game refactor. Chase H.Q. remains the sole authoritative runtime.

## Guardrails
1. Existing Chase H.Q. execution and addresses remain unchanged.
2. Profiles never activate a non-ChaseHQ runtime in v1.
3. Cross-game work is a time-boxed validation experiment.
4. Generalisation is accepted only when it improves or validates Chase H.Q. tooling.
5. Unsupported capabilities must be reported as unknown/unmapped, never guessed.

## API
- `GET /api/v1/profiles` lists descriptors.
- `GET /api/v1/profile` returns the active Chase H.Q. descriptor.
- `GET /api/v1/profile?game=sci` returns the SCI experimental descriptor without changing runtime state.
- Script equivalents: `api profiles.list`, `api profile.get id=sci`, `getprofiles()`.

## Future probe
A future SCI probe, if deliberately enabled, should measure generic API/script compatibility and the number of ChaseHQ-specific assumptions encountered. It should not become a parallel port.
