if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

# ---------------------------------------------------------------------------------------
# Architecture
# ---------------------------------------------------------------------------------------
if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
  message(STATUS "Architecture: x86_64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
  message(STATUS "Architecture: AMD64")
else()
  message(
    FATAL_ERROR
      "The architecture does not support: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

# ---------------------------------------------------------------------------------------
# System
# ---------------------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
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
message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")

# ---------------------------------------------------------------------------------------
# Compiler
# ---------------------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.1.0)
    message(
      FATAL_ERROR
        "GCC version must be at least 11.1.0, the current version is: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  message(
    STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0.0)
    message(
      FATAL_ERROR
        "Clang version must be at least 12.0.0, the current version is: ${CMAKE_CXX_COMPILER_VERSION}"
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

if(CMAKE_COMPILER_IS_GNUCXX AND KLIB_BUILD_FUZZER)
  message(FATAL_ERROR "GCC does not support libFuzzer")
endif()

if(KLIB_VALGRIND AND KLIB_SANITIZER)
  message(FATAL_ERROR "Valgrind and sanitizer cannot be used at the same time ")
endif()
