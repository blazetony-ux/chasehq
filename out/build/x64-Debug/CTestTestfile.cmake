# CMake generated Testfile for 
# Source directory: C:/Projects/ChaseHQ-Native-v2-NoGit/ChaseHQ-Native-v0.8
# Build directory: C:/Projects/ChaseHQ-Native-v2-NoGit/ChaseHQ-Native-v0.8/out/build/x64-Debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[cpu_bus_rom_tests]=] "C:/Projects/ChaseHQ-Native-v2-NoGit/ChaseHQ-Native-v0.8/out/build/x64-Debug/runtime_tests.exe")
set_tests_properties([=[cpu_bus_rom_tests]=] PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "C:/Projects/ChaseHQ-Native-v2-NoGit/ChaseHQ-Native-v0.8/CMakeLists.txt;40;add_test;C:/Projects/ChaseHQ-Native-v2-NoGit/ChaseHQ-Native-v0.8/CMakeLists.txt;0;")
subdirs("SDL")
