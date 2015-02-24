enable_testing()

file(GLOB TEST_SRC_FILES ${CMAKE_SOURCE_DIR}/tests/unit-tests/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/gtest/include)

foreach(_BUILD_TYPE ${KINARA_BUILD_TYPES})
  set(_TARGET_NAME kinara-unit-tests.${_BUILD_TYPE})
  set(_KINARA_LIBS_TO_LINK "")
  foreach(_KINARA_LIB ${KINARA_LIB_PROJECTS})
    set(_KINARA_LIBS_TO_LINK "${_KINARA_LIB}.${_BUILD_TYPE};${_KINARA_LIBS_TO_LINK}")
  endforeach(_KINARA_LIB)

  add_executable(${_TARGET_NAME} ${TEST_SRC_FILES})
  add_dependencies(${_TARGET_NAME} gtest)
  add_dependencies(${_TARGET_NAME} ${_KINARA_LIBS_TO_LINK})

  target_link_libraries(${_TARGET_NAME} gtest gtest_main)

  target_link_libraries(${_TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(${_TARGET_NAME} ${_KINARA_LIBS_TO_LINK})
  if(KINARA_CFG_HAVE_LIBRT_)
    target_link_libraries(${_TARGET_NAME} rt)
  endif()

  if(_TARGET_NAME MATCHES "\\.lto")
    set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "-O3;-flto")
    set_target_properties(${_TARGET_NAME} PROPERTIES LINK_FLAGS "-O3 -flto")
  elseif(_TARGET_NAME MATCHES "\\.opt")
    set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "-O3")
  else()
    set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS
      "-ggdb3;-O0;-fno-inline;-DKINARA_CFG_DEBUG_MODE_BUILD_")
  endif()

  set_target_properties(${_TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
    )


  add_test(unit_test ${_TARGET_NAME})
endforeach(_BUILD_TYPE)
