cmake_minimum_required(VERSION 3.9.4)

project(aoc-farm-auto-reseed
  LANGUAGES C
)

if (UNIX)
  set(CMAKE_CROSS_COMPILING 1)
  set(CMAKE_SYSTEM_NAME Windows)
  set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
  set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
  set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
endif()

if (NOT MSVC)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

if(NOT MSVC)
  add_compile_options(-m32)
  add_compile_options(-Wall)
endif()
add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)

include_directories(include/)

set(SOURCE_FILES
  hook.c
  main.c
  farm_auto_reseed.c
  main.def
)

add_library(aoc-farm-auto-reseed SHARED ${SOURCE_FILES})

if (UNIX)
  set_target_properties(aoc-farm-auto-reseed PROPERTIES
      PREFIX ""
      SUFFIX ".dll")
endif()
install(TARGETS aoc-farm-auto-reseed
        LIBRARY DESTINATION lib)
