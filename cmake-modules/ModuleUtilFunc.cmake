function(define_lib_module module-name)
  set(_MODULE_SOURCES ${ARGN})

  foreach(BUILD_SUFFIX ${KINARA_BUILD_TYPES})
    set(_TARGET_NAME "${module-name}.${BUILD_SUFFIX}")

    add_library(${_TARGET_NAME} OBJECT ${_MODULE_SOURCES})
    if(_TARGET_NAME MATCHES "\\.shared")
      set_property(TARGET ${_TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()
    if(_TARGET_NAME MATCHES "\\.lto")
      if(KINARA_DEBUG_INFO)
        set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "-O3;-flto;-ggdb3")
      else()
        set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "-O3;-flto")
      endif()
    elseif(_TARGET_NAME MATCHES "\\.opt")
      if(KINARA_DEBUG_INFO)
        set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "-O3;-ggdb3")
      else()
        set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS "-O3")
      endif()
    else()
      set_target_properties(${_TARGET_NAME} PROPERTIES COMPILE_OPTIONS
        "-ggdb3;-O0;-fno-inline;-DKINARA_CFG_DEBUG_MODE_BUILD_")
    endif()
  endforeach(BUILD_SUFFIX)

endfunction(define_lib_module)
