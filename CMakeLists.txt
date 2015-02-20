cmake_minimum_required(VERSION 2.8)
project(kinara)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

set(CMAKE_BUILD_TYPE None CACHE STRING "Build Type" FORCE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

set(KINARA_BIN_DIR "bin")
set(KINARA_LIB_DIR "lib")
set(KINARA_LIB_DIR "include")

include(ConfigureCompiler)
include(ModuleUtilFunc)

# set the kinds of builds we want
set(KINARA_BUILD_TYPES
  debug-shared
  debug-static
  debug-log-shared
  debug-log-static
  opt-shared
  opt-static
  opt-log-shared
  opt-log-static
)

if(KINARA_LTO_BUILDS)
  LIST(APPEND KINARA_BUILD_TYPES
    lto-shared
    lto-static
    lto-log-shared
    lto-log-static
)
endif()

#definitions of the top level projects
set(KINARA_LIB_PROJECTS
  kinara-common
)
# empty for now
set(KINARA_EXE_PROJECTS ""
)

set(KINARA_PROJECTS ${KINARA_LIB_PROJECTS} ${KINARA_EXE_PROJECTS})

# definitions for the modules
set(kinara-common-modules allocators containers hashfuncs primeutils)

# add the modules for kinara
foreach(_KINARA_PROJECT ${KINARA_PROJECTS})
  set(_PROJECT_MODULES "${${_KINARA_PROJECT}-modules}")
  foreach(_PROJECT_MODULE ${_PROJECT_MODULES})
    set(_CURRENT_SUBDIR ${CMAKE_SOURCE_DIR}/projects/${_KINARA_PROJECT}/src/${_PROJECT_MODULE})
    add_subdirectory(${_CURRENT_SUBDIR})
  endforeach(_PROJECT_MODULE)
endforeach(_KINARA_PROJECT)

foreach(_KINARA_LIB_PROJECT ${KINARA_LIB_PROJECTS})
  foreach(_BUILD_TYPE ${KINARA_BUILD_TYPES})
    set(_MODULE_LIST ${${_KINARA_LIB_PROJECT}-modules})

    set(_MODULES_FOR_BUILD_TYPE "")
    foreach(_MODULE ${_MODULE_LIST})
      list(APPEND _MODULES_FOR_BUILD_TYPE "$<TARGET_OBJECTS:${_KINARA_LIB_PROJECT}-${_MODULE}-${_BUILD_TYPE}>")
    endforeach(_MODULE)

    set(_TARGET_NAME "${_KINARA_LIB_PROJECT}-${_BUILD_TYPE}")

    if(_BUILD_TYPE MATCHES "-shared")
      add_library(${_TARGET_NAME} SHARED ${_MODULES_FOR_BUILD_TYPE})
      set_property(TARGET ${_TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    else()
      add_library(${_TARGET_NAME} STATIC ${_MODULES_FOR_BUILD_TYPE})
    endif()

    if(_TARGET_NAME MATCHES "-lto")
      set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "${CMAKE_CXX_FLAGS} -O3 -flto")
      set_target_properties(${_TARGET_NAME} PROPERTIES LINK_FLAGS "-O3 -flto")
    elseif(_TARGET_NAME MATCHES "-opt")
      set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "${CMAKE_CXX_FLAGS} -O3")
    else()
      set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "${CMAKE_CXX_FLAGS} -ggdb3 -O0 -fno-inline")
    endif()
  endforeach(_BUILD_TYPE)
endforeach(_KINARA_LIB_PROJECT)

foreach(_BUILD_TYPE ${KINARA_BUILD_TYPES})
  set(_DEP_TARGETS "")

  foreach(_KINARA_PROJECT ${KINARA_PROJECTS})
    set(_DEP_TARGETS "${_KINARA_PROJECT}-${_BUILD_TYPE}")
  endforeach(_KINARA_PROJECT)

  add_custom_target(${_BUILD_TYPE} DEPENDS ${_DEP_TARGETS})
endforeach(_BUILD_TYPE)