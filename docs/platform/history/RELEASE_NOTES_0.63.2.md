# ChaseHQ-Native v0.63.2 - Script Automation Patch

Built from the validated v0.63.1 workflow patch.

## Added

- Research Script v2 with variables, list variables, `for`, `repeat`, `${name}` substitution and bounded nesting.
- deterministic `wait-until step-complete`.
- `require capability`, `assert status`, `echo`, `help` and `api actions`.
- generic safe `zip evidence ...` and `zip ioc-sweep ...` verbs.
- named `api ACTION key=value` layer so safe Research API capabilities can be automated from the Script Console.
- persistent Script Library under `research/scripts/`, with Workbench list/load/save controls.
- bundled reusable baseline, IOC, evidence and handling scripts.
- script transcript loop context (`mask=04`, `repeat=2/5`, etc.).
- IOC sweep core shared between HTTP endpoint and script action.
- evidence capture core shared between HTTP endpoint and script action.

## Philosophy

Once the Workbench is running, normal safe research instructions should prefer Script Console automation. UI/API features should not become isolated one-off workflows.

## Retained fixes

All v0.63.1 timer-hold, evidence, controller calibration, palette restoration, checkpoint and Web-host fixes are carried forward.
