cmake_minimum_required(VERSION 2.8)
project(kinara)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Type" FORCE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "default install path" FORCE)
endif()

set(KINARA_BIN_DIR "bin")
set(KINARA_LIB_DIR "lib")
set(KINARA_LIB_DIR "include")

include(ConfigureCompiler)

# add the modules for kinara
add_subdirectory("${CMAKE_SOURCE_DIR}/projects/kinara-common/src/allocators")
# add_subdirectory("${CMAKE_SOURCE_DIR}/projects/kinara-common/src/basetypes")
# add_subdirectory("${CMAKE_SOURCE_DIR}/projects/kinara-common/src/containers")
# add_subdirectory("${CMAKE_SOURCE_DIR}/projects/kinara-common/src/hashfuncs")
# add_subdirectory("${CMAKE_SOURCE_DIR}/projects/kinara-common/src/memory")
# add_subdirectory("${CMAKE_SOURCE_DIR}/projects/kinara-common/src/primeutils")

add_library(kinara-common SHARED
  $<TARGET_OBJECTS:kinara-common-allocators>
)
