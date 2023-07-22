cmake_minimum_required(VERSION 3.10)
include(CheckCXXCompilerFlag)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(CCACHE_PROGRAM ccache)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(CCACHE_PROGRAM)
  message(STATUS "Using compiler cache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()
project(KaHIP C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# if no build mode is specified build in release mode
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# tweak compiler flags
CHECK_CXX_COMPILER_FLAG(-funroll-loops COMPILER_SUPPORTS_FUNROLL_LOOPS)
if(COMPILER_SUPPORTS_FUNROLL_LOOPS)
  add_definitions(-funroll-loops)
endif()
CHECK_CXX_COMPILER_FLAG(-fno-stack-limit COMPILER_SUPPORTS_FNOSTACKLIMITS)
if(COMPILER_SUPPORTS_FNOSTACKLIMITS)
  add_definitions(-fno-stack-limit)
endif()
CHECK_CXX_COMPILER_FLAG(-Wall COMPILER_SUPPORTS_WALL)
if(COMPILER_SUPPORTS_WALL)
  add_definitions(-Wall)
endif()
CHECK_CXX_COMPILER_FLAG(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
  add_definitions(-march=native)
endif()
CHECK_CXX_COMPILER_FLAG(-fpermissive COMPILER_SUPPORTS_FPERMISSIVE)
if(COMPILER_SUPPORTS_FPERMISSIVE)
  add_definitions(-fpermissive)
endif()

# check dependencies
find_package(MPI REQUIRED)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  message(STATUS "OpenMP support detected")
  add_definitions(${OpenMP_CXX_FLAGS})
else()
  message(WARNING "OpenMP not available, activating workaround")
  add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
  set_property(TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_COMPILE_OPTIONS "")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/misc)
endif()

# 64 Bit option
#option(64BITMODE "64 bit mode" OFF)
option(64BITMODE "64 bit mode" ON)
if(64BITMODE)
  add_definitions("-DMODE64BITEDGES")
  add_definitions("-DPOINTER64=1")
endif()

# optimized output
option(OPTIMIZED_OUTPUT "optimized output" OFF)
if(OPTIMIZED_OUTPUT)
  add_definitions("-DKAFFPAOUTPUT")
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/argtable3-3.0.3)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/io)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/partition)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/tools)



set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc")

set(LIBKAFFPA_SOURCE_FILES
  lib/tools/quality_metrics.cpp
  lib/tools/random_functions.cpp
  extern/argtable3-3.0.3/argtable3.c)
add_library(libkaffpa OBJECT ${LIBKAFFPA_SOURCE_FILES})

set(LIBMAPPING_SOURCE_FILES
  lib/mapping/construct_distance_matrix.cpp
  )
add_library(libmapping OBJECT ${LIBMAPPING_SOURCE_FILES})

set(LIBSTREAMPARTITIONING_SOURCE_FILES
  lib/io/graph_io_stream.cpp
  lib/partition/onepass_partitioning/vertex_partitioning.cpp
  lib/partition/onepass_partitioning/floating_block.cpp
  lib/partition/onepass_partitioning/ldg.cpp
  lib/partition/onepass_partitioning/fennel_approx_sqrt.cpp
  lib/partition/onepass_partitioning/fennel.cpp)
add_library(libstreanpartitioning OBJECT ${LIBSTREAMPARTITIONING_SOURCE_FILES})

# generate targets for each binary
add_executable(freight_cut app/freight.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping> $<TARGET_OBJECTS:libstreanpartitioning>)
target_compile_definitions(freight_cut PRIVATE "-DMODE_NETLIST")
target_compile_definitions(freight_cut PRIVATE "-DMODE_FREIGHT")
target_link_libraries(freight_cut ${OpenMP_CXX_LIBRARIES})
install(TARGETS freight_cut DESTINATION bin)

add_executable(freight_con app/freight.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping> $<TARGET_OBJECTS:libstreanpartitioning>)
target_compile_definitions(freight_con PRIVATE "-DMODE_NETLIST")
target_compile_definitions(freight_con PRIVATE "-DMODE_FREIGHT")
target_compile_definitions(freight_con PRIVATE "-DMODE_CONNECTIVITY")
target_link_libraries(freight_con ${OpenMP_CXX_LIBRARIES})
install(TARGETS freight_con DESTINATION bin)


