cmake_minimum_required(VERSION 3.0.0)
project(BurstingGraphMining)
SET(NLOPT_LIBRARY "/usr/local/lib/libnlopt.so")
SET(NLOPT_LIBRARIES "/usr/local/lib/libnlopt.so.0" "/usr/local/lib/libnlopt.so.0.10.0")

find_package(Boost REQUIRED COMPONENTS graph)
find_package(NLopt REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

# Create a static library from burst_graph.cpp
add_library(burst_graph STATIC burst_graph/burst_graph.cpp)

# Link the burst_graph library to your executables
add_executable(rush main.cpp)
target_link_libraries(rush burst_graph ${Boost_LIBRARIES})

add_executable(load_test test/load_graph_test.cpp)
target_link_libraries(load_test burst_graph ${Boost_LIBRARIES})

add_executable(peeling_test test/peeling_test.cpp)
target_link_libraries(peeling_test burst_graph ${Boost_LIBRARIES})

add_executable(output_test test/output_dense_subgraph.cpp)
target_link_libraries(output_test burst_graph ${Boost_LIBRARIES})
