if(NOT DEFINED KLIB_MAIN_PROJECT)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(KLIB_MAIN_PROJECT ON)
  else()
    set(KLIB_MAIN_PROJECT OFF)
  endif()
endif()

option(KLIB_BUILD_STATIC "Build static library" ON)
option(KLIB_BUILD_SHARED "Build shared library" ON)

option(KLIB_BUILD_TEST "Build test" OFF)
option(KLIB_BUILD_EXTRA_TEST "Build extra test" OFF)

option(KLIB_BUILD_BENCH "Build benchmark" OFF)
option(KLIB_BUILD_DOC "Build documentation" OFF)

option(KLIB_FORMAT "Format code using clang-format and cmake-format" OFF)
option(KLIB_CLANG_TIDY "Analyze code with clang-tidy" OFF)
option(KLIB_SANITIZER "Build with AddressSanitizer and UndefinedSanitizer" OFF)

option(KLIB_INSTALL "Generate the install target" ${KLIB_MAIN_PROJECT})

include(CMakeDependentOption)
cmake_dependent_option(
  KLIB_BUILD_COVERAGE "Build test with coverage information" OFF
  "BUILD_TESTING;KLIB_BUILD_TEST" OFF)
cmake_dependent_option(KLIB_VALGRIND "Execute test with valgrind" OFF
                       "BUILD_TESTING;KLIB_BUILD_TEST" OFF)
