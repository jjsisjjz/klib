enable_testing()

add_subdirectory(unit_test)

if(KLIB_BUILD_EXTRA_TEST)
  message(STATUS "Build extra test")
  add_subdirectory(extra_test)
endif()

include(Coverage)
include(Valgrind)
