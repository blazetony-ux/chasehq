# v0.58.1 — MSVC optimized-Debug compile fix

v0.58.0 deliberately enables `/O2` on the Musashi core and ChaseHQ runtime/native targets in the normal `x64-Debug` workflow so fast-forward is not interpreter-bound by an unoptimised Debug build.

MSVC's default CMake Debug flags also contain `/RTC1`. MSVC rejects `/RTC1` together with `/O2` (D8016), so v0.58.0 could not compile on the Windows/MSVC workflow.

v0.58.1 removes `/RTC*` from the MSVC Debug baseline before applying target-local `/O2`. Debug symbols and the Debug CRT remain enabled. The build-time `m68kmake` generator is still explicitly `/Od` because the existing MSVC 19.44 generator workaround must remain intact.

The remaining `/Od` baseline may produce D9025 "overriding /Od with /O2" warnings on the deliberately optimised targets; that override is intentional and is not a build failure.
