execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
  OUTPUT_VARIABLE CXX_COMPILER_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REGEX MATCH "[0-9]+(\\.[0-9]+)*" CXX_COMPILER_VERSION ${CXX_COMPILER_VERSION})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if(CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
    message(FATAL_ERROR "Kinara requires g++ 4.8.1 or later")
  else()
    message(STATUS "Kinara: Using g++ ${CXX_COMPILER_VERSION} as the c++ compiler in C++11 mode")
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CXX_COMPILER_VERSION VERSION_LESS "3.3")
    message(FATAL_ERROR, "Kinara requires clang++ 3.3 or later")
  else()
    message(STATUS "Kinara: Using clang++ ${CXX_COMPILER_VERSION} as the c++ compiler in C++11 mode")
  endif()
else()
  message(FATAL_ERROR "Unsupported c++ compiler. Only g++ and clang are currently supported by Kinara")
endif()

# set standard cxxflags
set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Wall -pipe")

# run the tests to determine configuration
include(RunConfigurationTests)
