cmake_minimum_required(VERSION 3.10)

project(DPDK_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -g -O3 -DLOG -march=native -mno-avx512f")

file(GLOB lib_src lib/*)

file(GLOB app_src    app/*)
file(GLOB server_src server/*)
file(GLOB logger_src logger/*)

include_directories(lib)
link_directories(/usr/local/lib/x86_64-linux-gnu /usr/local/lib)

add_library(lib STATIC ${lib_src})
target_link_libraries(lib PUBLIC pthread rte_eal rte_ethdev rte_mbuf rte_mempool rte_ring InfluxDB)
link_libraries(lib)

add_executable(app    ${app_src})
add_executable(server ${server_src})
add_executable(logger ${logger_src})

