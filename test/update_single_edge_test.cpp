#include "burst_graph.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

int main() {
    BurstGraph<false> graph;  // Assuming directed graph; replace 'true' with 'false' if needed.

    // Load graph from a file
    std::string filename = "/home/yuhang/Work/BurstingGraphMining/test.txt";
    if (!graph.LoadGraphFromFile(filename)) {
        std::cerr << "Error loading graph from file." << std::endl;
        return 1;
    }

    // File with multiple edges
    std::string edgesFilename = "/home/yuhang/Work/BurstingGraphMining/test4.txt";
    std::ifstream edgesFile(edgesFilename);
    if (!edgesFile.is_open()) {
        std::cerr << "Error opening edges file." << std::endl;
        return 1;
    }

    std::string line;
    int count = 0;
    double total_time = 0.0;

    // Generate the peeling sequence
    graph.GeneratePeelingSequence();
    graph.FindDenseSubgraph();

    while (std::getline(edgesFile, line)) {
        std::istringstream iss(line);
        std::string source_vertex, target_vertex;
        double weight;
        int timestamp;

        if (!(iss >> source_vertex >> target_vertex >> weight >> timestamp)) {
            std::cerr << "Error reading edge from file." << std::endl;
            continue;
        }

        auto start = std::chrono::high_resolution_clock::now();

        graph.AddOneEdge(source_vertex, target_vertex, weight, timestamp);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        // std::cout << elapsed.count() << std::endl;
        
        total_time += elapsed.count();
        count++;
    }

    edgesFile.close();

    std::cout << "Processed " << count << " edges." << std::endl;
    std::cout << "Average time per update: " << total_time / count << " ms." << std::endl;

    graph.FindDenseSubgraph();

    graph.OutputDenseSubgraph("Dense_subgraph.txt");

    return 0;
}