#include <iostream>
#include "burst_graph.hpp"

int main() {
    // Create a BurstGraph object
    BurstGraph<true> burstGraph;

    // Load the graph from the input file (e.g., "input.txt")
    if (!burstGraph.LoadGraphFromFile("/home/yuhang/grab_data/burst_test/ios_data_first.txt", 1)) {
        std::cerr << "Error: Failed to load the graph from the file." << std::endl;
        return 1;
    }

    // Get the loaded graph
    const auto& graph = burstGraph.GetGraph();

    // Print the vertices and edges of the loaded graph
    std::cout << "Vertices: ";
    for (auto vertexIt = boost::vertices(graph).first; vertexIt != boost::vertices(graph).second; ++vertexIt) {
        std::cout << graph[*vertexIt].id << " ";
    }
    std::cout << std::endl;

    std::cout << "Edges: ";
    for (auto edgeIt = boost::edges(graph).first; edgeIt != boost::edges(graph).second; ++edgeIt) {
        auto sourceID = graph[boost::source(*edgeIt, graph)].id;
        auto destID = graph[boost::target(*edgeIt, graph)].id;
        std::cout << "(" << sourceID << " -> " << destID << ") ";
    }
    std::cout << std::endl;

    return 0;
}
