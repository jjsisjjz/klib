# ---------------------------------------------------------------------------------------
# In-source build
# ---------------------------------------------------------------------------------------
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-source build is not allowed")
endif()

# ---------------------------------------------------------------------------------------
# Architecture
# ---------------------------------------------------------------------------------------
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
  message(STATUS "Architecture: x86_64")
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
  message(STATUS "Architecture: AMD64")
else()
  message(
    FATAL_ERROR
      "The architecture does not support: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

# ---------------------------------------------------------------------------------------
# System
# ---------------------------------------------------------------------------------------
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  # https://kurotych.com/development/cmake_check_linux_kernel_version/
  execute_process(
    COMMAND uname -r
    OUTPUT_VARIABLE KERNEL_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${KERNEL_VERSION}")
else()
  message(FATAL_ERROR "The system does not support: ${CMAKE_SYSTEM_NAME}")
endif()

# ---------------------------------------------------------------------------------------
# Generator
# ---------------------------------------------------------------------------------------
if(${CMAKE_GENERATOR} STREQUAL "Ninja")
  execute_process(
    COMMAND ninja --version
    OUTPUT_VARIABLE GENERATOR_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" GENERATOR_VERSION ${GENERATOR_VERSION})
  list(GET GENERATOR_VERSION 0 GENERATOR_VERSION)

  message(STATUS "CMake Generator: Ninja ${GENERATOR_VERSION}")
else()
  message(WARNING "The generator recommends using Ninja: ${CMAKE_GENERATOR}")
endif()

# ---------------------------------------------------------------------------------------
# Compiler
# ---------------------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
  if(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 11.1.0)
    message(
      FATAL_ERROR
        "GCC version must be at least 11.1.0, the current version is: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  message(
    STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  if(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 13.0.0)
    message(
      FATAL_ERROR
        "Clang version must be at least 13.0.0, the current version is: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  message(
    STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
else()
  message(FATAL_ERROR "The compiler does not support: ${CMAKE_CXX_COMPILER_ID}")
endif()

# ---------------------------------------------------------------------------------------
# Option
# ---------------------------------------------------------------------------------------
if(NOT (KLIB_BUILD_STATIC OR KLIB_BUILD_SHARED))
  message(FATAL_ERROR "You need to build at least one flavor of klib")
endif()

if((${CMAKE_BUILD_TYPE} STREQUAL "Debug") AND KLIB_BUILD_BENCH)
  message(
    FATAL_ERROR "The CMAKE_BUILD_TYPE cannot be Debug when building benchmark")
endif()

if(KLIB_SANITIZER AND KLIB_VALGRIND)
  message(
    FATAL_ERROR "AddressSanitizer and valgrind cannot be used at the same time")
endif()

if(NOT (BUILD_TESTING AND KLIB_BUILD_TEST) AND KLIB_SANITIZER)
  message(FATAL_ERROR "Must build test when using AddressSanitizer")
endif()

if(NOT (BUILD_TESTING AND KLIB_BUILD_TEST) AND KLIB_VALGRIND)
  message(FATAL_ERROR "Must build test when using valgrind")
endif()
