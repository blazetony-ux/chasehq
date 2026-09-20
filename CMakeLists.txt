cmake_minimum_required(VERSION 3.20)
project(ChaseHQNative VERSION 0.8 LANGUAGES C CXX)
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
add_library(musashi STATIC "${MUSASHI}/m68kcpu.c"
    "${MUSASHI}/softfloat/softfloat.c" "${GENERATED}/m68kops.c")
target_include_directories(musashi PUBLIC "${MUSASHI}" PRIVATE "${GENERATED}")
target_compile_definitions(musashi PUBLIC
    M68K_EMULATE_010=0 M68K_EMULATE_EC020=0 M68K_EMULATE_020=0
    M68K_EMULATE_030=0 M68K_EMULATE_040=0 M68K_EMULATE_PMMU=0
    M68K_INSTRUCTION_HOOK=1 M68K_EMULATE_INT_ACK=1
    M68K_EMULATE_ADDRESS_ERROR=1 M68K_EMULATE_TRACE=1)
if(MSVC)
    # MSVC 19.44 /O2 miscompiles this upstream generator's opcode-mask parser.
    # Keep the build-time generator unoptimized; the emulation core stays optimized.
    target_compile_options(m68kmake PRIVATE /Od)
    target_compile_definitions(m68kmake PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(musashi PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_library(chq_runtime STATIC src/runtime.cpp src/cpu_rom.cpp)
target_include_directories(chq_runtime PUBLIC src)
target_link_libraries(chq_runtime PUBLIC musashi)
target_compile_definitions(chq_runtime PUBLIC
    CHASEHQ_NATIVE_VERSION="0.8" CHASEHQ_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(ChaseHQRuntime src/runtime_main.cpp)
target_link_libraries(ChaseHQRuntime PRIVATE chq_runtime)
if(BUILD_TESTING)
    add_executable(runtime_tests tests/runtime_tests.cpp)
    target_link_libraries(runtime_tests PRIVATE chq_runtime)
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
    add_executable(ChaseHQNative src/main.cpp src/machine.cpp src/rom_loader.cpp src/video.cpp)
    target_link_libraries(ChaseHQNative PRIVATE chq_runtime SDL3::SDL3)
    if(WIN32)
        add_custom_command(TARGET ChaseHQNative POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:SDL3::SDL3-shared>" "$<TARGET_FILE_DIR:ChaseHQNative>" VERBATIM)
    endif()
endif()
if(MSVC)
    target_compile_options(chq_runtime PRIVATE /W4 /permissive-)
endif()
