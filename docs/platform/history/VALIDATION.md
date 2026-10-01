# v0.59.3 validation

Container validation completed for CPU/runtime configure/build and unit tests. `src/main.cpp` also passed C++20 syntax validation against the bundled headers. The SDL3 source tree is intentionally not bundled, so the full SDL frontend cannot be linked in this container; Windows acceptance remains required using `out/build/x64-Debug/ChaseHQNative.exe`.

First Windows acceptance should verify corrected handling telemetry, then run controlled native-vs-override cornering A/B tests and a course-profile replay from the same checkpoint.
