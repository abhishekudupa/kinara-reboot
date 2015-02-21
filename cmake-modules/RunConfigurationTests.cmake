include(CheckLibraryExists)

message(STATUS "Kinara: Testing for 64 bit system...")
try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_SOURCE_DIR}/cmake-modules/cmake-tests/test-64-bit.cpp"
  COMPILE_DEFINITIONS "${CMAKE_CXX_FLAGS}")
if(_COMPILE_RESULT_VAR AND NOT _RUN_RESULT_VAR)
  message(STATUS "Kinara: 64 bit system test passed")
elseif(_COMPILE_RESULT_VAR AND _RUN_RESULT_VAR)
  message(FATAL_ERROR "Kinara: 64 bit system test failed. Kinara currently only supports 64-bit platforms")
else()
  message(FATAL_ERROR,"Kinara: 64 bit system test failed to compile")
endif()

message(STATUS "Kinara: Testing for SSE 4.2...")
execute_process(COMMAND "${CMAKE_SOURCE_DIR}/cmake-modules/cmake-tests/test-sse-4.2.py"
  RESULT_VARIABLE _RESULT_VARIABLE)
if(NOT _RESULT_VARIABLE)
  message(STATUS "Kinara: Platform supports SSE 4.2, enabling it")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
else()
  message(STATUS "Kinara: Platform does NOT support SSE 4.2, not using SSE 4.2")
endif()

message(STATUS "Kinara: Testing if the compiler can do link-time-optimized builds...")
try_run(_RUN_RESULT_VAR _COMPILE_RESULT_VAR "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_SOURCE_DIR}/cmake-modules/cmake-tests/test-lto.cpp"
  COMPILE_DEFINITIONS "${CMAKE_CXX_FLAGS} -flto")
if(NOT _COMPILE_RESULT_VAR)
  message(STATUS "Kinara: Link-time-optimization unsupported, disabling LTO builds")
  set(KINARA_LTO_BUILDS OFF)
elseif(_RUN_RESULT_VAR)
  message(STATUS "Kinara: Link-time-optimization produces bad outputs, disabling LTO builds")
  set(KINARA_LTO_BUILDS OFF)
else()
  message(STATUS "Kinara: Link-time-optimization supported, enabling LTO builds")
  set(KINARA_LTO_BUILDS ON)
endif()

message(STATUS "Kinara: Using base CXXFLAGS = \"${CMAKE_CXX_FLAGS}\"")

# find if we have a gdb
execute_process(COMMAND "which" "gdb" RESULT_VARIABLE _RESULT_VARIABLE
  OUTPUT_VARIABLE _OUT_VAR ERROR_VARIABLE _OUT_VAR)
string(STRIP ${_OUT_VAR} _PATH_TO_GDB)
if(RESULT_VARIABLE)
  message(STATUS "Kinara: No gdb found, disabling debug feaures on all builds")
  set(KINARA_CFG_HAVE_GDB_ OFF)
  set(KINARA_CFG_PATH_TO_GDB_ "#error \"No GDB found, but tried to invoke debugger. Please contact maintainer\"")
else()
  message(STATUS "Kinara: Using gdb: \"${_PATH_TO_GDB}\" and enabling debug features on debug builds")
  set(KINARA_CFG_HAVE_GDB_ ON)
  set(KINARA_CFG_PATH_TO_GDB_ "\"${_PATH_TO_GDB}\"")
endif()

set(KINARA_CFG_HAVE_GDB_ OFF)
set(KINARA_CFG_PATH_TO_GDB_ "#error \"No GDB found, but tried to invoke debugger. Please contact maintainer\"")

message(STATUS "Kinara: Checking for librt")
CHECK_LIBRARY_EXISTS(rt timer_create "time.h" KINARA_CFG_HAVE_LIBRT_)
if(NOT KINARA_CFG_HAVE_LIBRT_)
  message(STATUS "Kinara: Could not find librt, all timer related functionality will be disabled!")
else()
  message(STATUS "Kinara: librt exists, all timer related functionality will be enabled!")
endif()
