cmake_minimum_required(VERSION 3.10)

project(DPDK_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -mssse3 -g -O0 -DLOG")

file(GLOB lib_src lib/*)

file(GLOB app_src    app/*)
file(GLOB server_src server/*)
file(GLOB logger_src logger/*)

include_directories($ENV{RTE_SDK}/$ENV{RTE_TARGET}/include lib)
link_directories($ENV{RTE_SDK}/$ENV{RTE_TARGET}/lib)

add_library(lib STATIC ${lib_src})
target_link_libraries(lib PUBLIC pthread rte_eal rte_ethdev rte_mbuf rte_mempool rte_ring)
link_libraries(lib)

add_executable(app    ${app_src})
add_executable(server ${server_src})
add_executable(logger ${logger_src})

