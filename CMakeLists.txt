cmake_minimum_required(VERSION 3.10)
include(CheckCXXCompilerFlag)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
  message(STATUS "Using compiler cache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

project(FREIGHT C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Default to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Compiler flags
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
option(NONATIVEOPTIMIZATIONS "Disable -march=native (for portable binaries)" OFF)
if(NOT NONATIVEOPTIMIZATIONS)
  CHECK_CXX_COMPILER_FLAG(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)
  if(COMPILER_SUPPORTS_MARCH_NATIVE)
    add_definitions(-march=native)
  endif()
endif()
CHECK_CXX_COMPILER_FLAG(-fpermissive COMPILER_SUPPORTS_FPERMISSIVE)
if(COMPILER_SUPPORTS_FPERMISSIVE)
  add_definitions(-fpermissive)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_definitions(-fconstexpr-steps=100000000)
endif()

# Dependencies
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  message(STATUS "OpenMP support detected")
  add_definitions(${OpenMP_CXX_FLAGS})
else()
  message(WARNING "OpenMP not available, activating workaround")
  add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
  set_property(TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_COMPILE_OPTIONS "")
endif()

# 64-bit mode
option(64BITMODE "64 bit mode" ON)
if(64BITMODE)
  add_definitions("-DMODE64BITEDGES" "-DPOINTER64=1")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc")

# Helper: create targets for a code base with scoped include dirs
set(HG ${CMAKE_CURRENT_SOURCE_DIR}/code_for_hypergraphs)
set(GR ${CMAKE_CURRENT_SOURCE_DIR}/code_for_graphs)

set(HG_INCLUDES ${HG} ${HG}/app ${HG}/extern/argtable3-3.0.3 ${HG}/lib ${HG}/lib/io ${HG}/lib/partition ${HG}/lib/tools)
set(GR_INCLUDES ${GR} ${GR}/app ${GR}/extern/argtable3-3.0.3 ${GR}/lib ${GR}/lib/io ${GR}/lib/partition ${GR}/lib/tools)

# -------------------- Hypergraph targets --------------------
add_library(hg_kaffpa OBJECT
  ${HG}/lib/tools/quality_metrics.cpp
  ${HG}/lib/tools/random_functions.cpp
  ${HG}/extern/argtable3-3.0.3/argtable3.c)
target_include_directories(hg_kaffpa PRIVATE ${HG_INCLUDES})

add_library(hg_mapping OBJECT ${HG}/lib/mapping/construct_distance_matrix.cpp)
target_include_directories(hg_mapping PRIVATE ${HG_INCLUDES})

add_library(hg_streampart OBJECT
  ${HG}/lib/io/graph_io_stream.cpp
  ${HG}/lib/partition/onepass_partitioning/vertex_partitioning.cpp
  ${HG}/lib/partition/onepass_partitioning/floating_block.cpp
  ${HG}/lib/partition/onepass_partitioning/ldg.cpp
  ${HG}/lib/partition/onepass_partitioning/fennel_approx_sqrt.cpp
  ${HG}/lib/partition/onepass_partitioning/fennel.cpp)
target_include_directories(hg_streampart PRIVATE ${HG_INCLUDES})

add_executable(freight_cut ${HG}/app/freight.cpp
  $<TARGET_OBJECTS:hg_kaffpa> $<TARGET_OBJECTS:hg_mapping> $<TARGET_OBJECTS:hg_streampart>)
target_include_directories(freight_cut PRIVATE ${HG_INCLUDES})
target_compile_definitions(freight_cut PRIVATE "-DMODE_NETLIST" "-DMODE_FREIGHT")
target_link_libraries(freight_cut ${OpenMP_CXX_LIBRARIES})
install(TARGETS freight_cut DESTINATION bin)

add_executable(freight_con ${HG}/app/freight.cpp
  $<TARGET_OBJECTS:hg_kaffpa> $<TARGET_OBJECTS:hg_mapping> $<TARGET_OBJECTS:hg_streampart>)
target_include_directories(freight_con PRIVATE ${HG_INCLUDES})
target_compile_definitions(freight_con PRIVATE "-DMODE_NETLIST" "-DMODE_FREIGHT" "-DMODE_CONNECTIVITY")
target_link_libraries(freight_con ${OpenMP_CXX_LIBRARIES})
install(TARGETS freight_con DESTINATION bin)

# -------------------- Graph targets --------------------
add_library(gr_kaffpa OBJECT
  ${GR}/lib/tools/quality_metrics.cpp
  ${GR}/lib/tools/random_functions.cpp
  ${GR}/extern/argtable3-3.0.3/argtable3.c)
target_include_directories(gr_kaffpa PRIVATE ${GR_INCLUDES})

add_library(gr_mapping OBJECT ${GR}/lib/mapping/construct_distance_matrix.cpp)
target_include_directories(gr_mapping PRIVATE ${GR_INCLUDES})

add_library(gr_streampart OBJECT
  ${GR}/lib/io/graph_io_stream.cpp
  ${GR}/lib/partition/onepass_partitioning/vertex_partitioning.cpp
  ${GR}/lib/partition/onepass_partitioning/floating_block.cpp
  ${GR}/lib/partition/onepass_partitioning/ldg.cpp
  ${GR}/lib/partition/onepass_partitioning/fennel_approx_sqrt.cpp
  ${GR}/lib/partition/onepass_partitioning/fennel.cpp)
target_include_directories(gr_streampart PRIVATE ${GR_INCLUDES})

add_executable(freight_graphs ${GR}/app/streammultisection.cpp
  $<TARGET_OBJECTS:gr_kaffpa> $<TARGET_OBJECTS:gr_mapping> $<TARGET_OBJECTS:gr_streampart>)
target_include_directories(freight_graphs PRIVATE ${GR_INCLUDES})
target_compile_definitions(freight_graphs PRIVATE "-DMODE_FREIGHT_GRAPHS")
target_link_libraries(freight_graphs ${OpenMP_CXX_LIBRARIES})
install(TARGETS freight_graphs DESTINATION bin)

# -------------------- Conversion tools --------------------
add_executable(hmetis_to_freight tools/hmetis_to_freight.cpp)
install(TARGETS hmetis_to_freight DESTINATION bin)

add_executable(hmetis_to_freight_stream tools/hmetis_to_freight_stream.cpp)
install(TARGETS hmetis_to_freight_stream DESTINATION bin)
