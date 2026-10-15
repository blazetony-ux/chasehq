cmake_minimum_required(VERSION 3.20)
project(ChaseHQNative VERSION 0.67.0.0 LANGUAGES C CXX)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h" CHQ_VERSION_LINE REGEX "kNativeVersion =")
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+(-RC[0-9]+(\\.[0-9]+)*)?" CHQ_RELEASE_VERSION "${CHQ_VERSION_LINE}")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(CHASEHQ_BUILD_SDL "Build preserved SDL3 scene frontend" ON)
include(CTest)
set(MUSASHI "${CMAKE_CURRENT_SOURCE_DIR}/third_party/musashi")
add_executable(m68kmake "${MUSASHI}/m68kmake.c")
set(GENERATED "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED}")
add_custom_command(OUTPUT "${GENERATED}/m68kops.c" "${GENERATED}/m68kops.h"
    COMMAND $<TARGET_FILE:m68kmake> "${GENERATED}" "${MUSASHI}/m68k_in.c"
    DEPENDS m68kmake "${MUSASHI}/m68k_in.c" VERBATIM)
add_library(musashi STATIC "${MUSASHI}/m68kcpu.c" "${MUSASHI}/m68kdasm.c"
    "${MUSASHI}/softfloat/softfloat.c" "${GENERATED}/m68kops.c")
target_include_directories(musashi PUBLIC "${MUSASHI}" PRIVATE "${GENERATED}")
target_compile_definitions(musashi PUBLIC
    M68K_EMULATE_010=0 M68K_EMULATE_EC020=0 M68K_EMULATE_020=0
    M68K_EMULATE_030=0 M68K_EMULATE_040=0 M68K_EMULATE_PMMU=0
    M68K_INSTRUCTION_HOOK=1 M68K_EMULATE_INT_ACK=1
    M68K_EMULATE_ADDRESS_ERROR=1 M68K_EMULATE_TRACE=1)
if(MSVC)
    # CMake's MSVC Debug flags include /RTC1. MSVC rejects every optimized
    # translation unit that also receives /O2 (D8016), so v0.59.1 retains the validated v0.58.3 behaviour and removes
    # /RTC from the Debug baseline before selectively enabling /O2 below.
    # /Od remains in the baseline; the later target-local /O2 intentionally
    # overrides it for the emulation/runtime hot paths.
    string(REGEX REPLACE "(^| )[/-]RTC[1csu]+" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REGEX REPLACE "(^| )[/-]RTC[1csu]+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    # Avoid D9025 in x64-Debug: remove CMake's explicit /Od baseline before
    # selectively adding /O2 to hot targets. Targets without /O2 still use
    # MSVC's normal unoptimised default.
    string(REGEX REPLACE "(^| )[/-]Od( |$)" " " CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REGEX REPLACE "(^| )[/-]Od( |$)" " " CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    # MSVC 19.44 /O2 miscompiles this upstream generator's opcode-mask parser.
    # Keep the build-time generator unoptimized; the emulation core stays optimized.
    target_compile_options(m68kmake PRIVATE /Od)
    target_compile_definitions(m68kmake PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(musashi PRIVATE _CRT_SECURE_NO_WARNINGS)
    # v0.58: our normal Windows workflow uses x64-Debug, but an unoptimised
    # interpreter makes deterministic fast-forward needlessly slow. Keep symbols
    # while optimising the hot emulation core/runtime in Debug builds.
    target_compile_options(musashi PRIVATE $<$<CONFIG:Debug>:/O2>)
endif()
add_library(chq_runtime STATIC src/runtime.cpp src/cpu_rom.cpp src/content_pack.cpp src/native_execution.cpp src/projection_shadow.cpp src/native_candidates.cpp
    src/native_sprite_clear.cpp src/live_candidate_shadow.cpp src/native_steering_conversion.cpp)
target_include_directories(chq_runtime PUBLIC src)
target_link_libraries(chq_runtime PUBLIC musashi)
if(CMAKE_DL_LIBS)
    target_link_libraries(chq_runtime PUBLIC ${CMAKE_DL_LIBS})
endif()
target_compile_definitions(chq_runtime PUBLIC
    CHASEHQ_NATIVE_VERSION="${CHQ_RELEASE_VERSION}" CHASEHQ_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(ChaseHQNativeEvidenceVerify src/native_candidate_verify.cpp src/native_candidates.cpp
    src/native_sprite_clear.cpp)
target_include_directories(ChaseHQNativeEvidenceVerify PRIVATE src)
add_executable(ChaseHQNativeSpriteEvidenceVerify src/native_candidate_verify_sprite.cpp src/native_candidates.cpp
    src/native_sprite_clear.cpp)
target_include_directories(ChaseHQNativeSpriteEvidenceVerify PRIVATE src)
add_executable(ChaseHQRuntime src/runtime_main.cpp)
target_link_libraries(ChaseHQRuntime PRIVATE chq_runtime)
add_executable(chqimport src/chqimport.cpp src/cpu_rom.cpp src/rom_loader.cpp src/content_pack.cpp)
target_include_directories(chqimport PRIVATE src)

add_library(chq_debug_api STATIC src/debug_api.cpp)
target_include_directories(chq_debug_api PUBLIC src)
if(WIN32)
    target_link_libraries(chq_debug_api PUBLIC ws2_32)
endif()
add_executable(chqctl src/chqctl.cpp)
if(WIN32)
    target_link_libraries(chqctl PRIVATE ws2_32)
endif()
if(BUILD_TESTING)
    add_executable(native_steering_conversion_tests tests/native_steering_conversion_tests.cpp src/native_steering_conversion.cpp)
    target_include_directories(native_steering_conversion_tests PRIVATE src)
    add_test(NAME native_steering_reference_193 COMMAND native_steering_conversion_tests "${CMAKE_CURRENT_SOURCE_DIR}/docs/reference/steering_reference_193.csv")
    add_executable(live_candidate_shadow_tests tests/live_candidate_shadow_tests.cpp src/native_candidates.cpp
    src/native_sprite_clear.cpp src/live_candidate_shadow.cpp src/native_steering_conversion.cpp)
    target_include_directories(live_candidate_shadow_tests PRIVATE src)
    add_test(NAME live_candidate_shadow_tests COMMAND live_candidate_shadow_tests)
    add_executable(native_sprite_clear_tests tests/native_sprite_clear_tests.cpp src/native_sprite_clear.cpp)
    target_include_directories(native_sprite_clear_tests PRIVATE src)
    add_test(NAME native_sprite_clear_tests COMMAND native_sprite_clear_tests)
    add_executable(native_candidates_tests tests/native_candidates_tests.cpp src/native_candidates.cpp
    src/native_sprite_clear.cpp)
    target_include_directories(native_candidates_tests PRIVATE src)
    add_test(NAME native_candidates_tests COMMAND native_candidates_tests)
    add_executable(projection_shadow_tests tests/projection_shadow_tests.cpp src/projection_shadow.cpp)
    target_include_directories(projection_shadow_tests PRIVATE src)
    add_test(NAME projection_shadow_tests COMMAND projection_shadow_tests)
    add_executable(native_execution_tests tests/native_execution_tests.cpp src/native_execution.cpp)
    target_include_directories(native_execution_tests PRIVATE src)
    add_test(NAME native_execution_tests COMMAND native_execution_tests)
    add_executable(content_pack_tests tests/content_pack_tests.cpp src/content_pack.cpp)
    target_include_directories(content_pack_tests PRIVATE src)
    add_test(NAME content_pack_tests COMMAND content_pack_tests)
    add_executable(tc0100scn_geometry_tests tests/tc0100scn_geometry_tests.cpp)
    target_include_directories(tc0100scn_geometry_tests PRIVATE src)
    add_test(NAME tc0100scn_geometry_tests COMMAND tc0100scn_geometry_tests)
    add_executable(sprite_priority_tests tests/sprite_priority_tests.cpp)
    target_include_directories(sprite_priority_tests PRIVATE src)
    add_test(NAME sprite_priority_tests COMMAND sprite_priority_tests)
    add_executable(sprite_gfx_tests tests/sprite_gfx_tests.cpp)
    target_include_directories(sprite_gfx_tests PRIVATE src)
    add_test(NAME sprite_gfx_tests COMMAND sprite_gfx_tests)
    add_executable(sprite_layer_shadow_tests tests/sprite_layer_shadow_tests.cpp)
    target_include_directories(sprite_layer_shadow_tests PRIVATE src)
    add_test(NAME sprite_layer_shadow_tests COMMAND sprite_layer_shadow_tests)
    add_executable(hud_layout_tests tests/hud_layout_tests.cpp)
    target_include_directories(hud_layout_tests PRIVATE src)
    add_test(NAME hud_layout_tests COMMAND hud_layout_tests)
    add_executable(tc0100scn_text_native_tests tests/tc0100scn_text_native_tests.cpp src/tc0100scn_text_native.cpp)
    target_include_directories(tc0100scn_text_native_tests PRIVATE src)
    add_test(NAME tc0100scn_text_native_tests COMMAND tc0100scn_text_native_tests)

    add_executable(tc0100scn_bg_native_tests tests/tc0100scn_bg_native_tests.cpp src/tc0100scn_bg_native.cpp)
    target_include_directories(tc0100scn_bg_native_tests PRIVATE src)
    add_test(NAME tc0100scn_bg_native_tests COMMAND tc0100scn_bg_native_tests)
    if(WIN32)
        add_test(NAME authoritative_history_tests COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/authoritative_history_tests.ps1")
        add_test(NAME sprite_snapshot_assertions COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/sprite_snapshot_assertions.ps1" -OutputRoot "${CMAKE_CURRENT_BINARY_DIR}/sprite-snapshot-tests")
    endif()
    # Synthetic target-health ABI fixture for the actual Musashi hook. This is
    # not the Ghidra-generated game function and never contributes to signoff.
    add_library(native_cpu_b_pair_stub SHARED tests/native_cpu_b_pair_stub.cpp)
    target_include_directories(native_cpu_b_pair_stub PRIVATE src)
    add_library(native_cpu_b_branch_stub SHARED tests/native_cpu_b_branch_stub.cpp)
    target_include_directories(native_cpu_b_branch_stub PRIVATE src)
    add_library(native_copy_boundary_stub SHARED tests/native_copy_boundary_stub.cpp)
    target_include_directories(native_copy_boundary_stub PRIVATE src)
    add_library(native_health_boundary_stub SHARED tests/native_health_boundary_stub.cpp)
    target_include_directories(native_health_boundary_stub PRIVATE src)
    add_executable(runtime_tests tests/runtime_tests.cpp)
    target_link_libraries(runtime_tests PRIVATE chq_runtime)
    target_compile_definitions(runtime_tests PRIVATE CHQ_HEALTH_TEST_LIBRARY_PATH="$<TARGET_FILE:native_health_boundary_stub>" CHQ_COPY_TEST_LIBRARY_PATH="$<TARGET_FILE:native_copy_boundary_stub>" CHQ_CPU_B_TEST_LIBRARY_PATH="$<TARGET_FILE:native_cpu_b_branch_stub>" CHQ_CPU_B_PAIR_TEST_LIBRARY_PATH="$<TARGET_FILE:native_cpu_b_pair_stub>")
    add_dependencies(runtime_tests native_health_boundary_stub native_copy_boundary_stub native_cpu_b_branch_stub native_cpu_b_pair_stub)
    add_test(NAME cpu_bus_rom_tests COMMAND runtime_tests)
    set_tests_properties(cpu_bus_rom_tests PROPERTIES TIMEOUT 30)
endif()
if(CHASEHQ_BUILD_SDL)
    set(CHASEHQ_SDL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL" CACHE PATH "Local SDL3 source tree")
    if(NOT EXISTS "${CHASEHQ_SDL_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "Copy your v0.7 SDL3 source into SDL/, or use -DCHASEHQ_BUILD_SDL=OFF for CPU-only operation.")
    endif()
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("${CHASEHQ_SDL_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/SDL" EXCLUDE_FROM_ALL)
    add_executable(ChaseHQNative src/main.cpp src/machine.cpp src/rom_loader.cpp src/video.cpp src/tc0100scn_text_native.cpp src/tc0100scn_bg_native.cpp)
    target_link_libraries(ChaseHQNative PRIVATE chq_runtime chq_debug_api SDL3::SDL3-shared)
    add_dependencies(ChaseHQNative SDL3-shared)
    if(WIN32)
        add_custom_command(TARGET ChaseHQNative POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:SDL3::SDL3-shared>" "$<TARGET_FILE_DIR:ChaseHQNative>" VERBATIM)
    endif()
endif()
if(MSVC)
    target_compile_options(chq_runtime PRIVATE /W4 /permissive- $<$<CONFIG:Debug>:/O2>)
    if(TARGET ChaseHQNative)
        target_compile_options(ChaseHQNative PRIVATE $<$<CONFIG:Debug>:/O2>)
    endif()
endif()

if(BUILD_TESTING AND WIN32 AND TARGET ChaseHQNative)
    add_test(NAME cli_evidence_tests COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_evidence_tests.ps1" -Exe "$<TARGET_FILE:ChaseHQNative>" -ExpectedBuild "${CHQ_RELEASE_VERSION}")
    set_tests_properties(cli_evidence_tests PROPERTIES TIMEOUT 60)
endif()
