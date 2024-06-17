#include <iostream>
#include <chrono> // For measuring time
#include "burst_graph.hpp" // Include your BurstGraph class definition

int main() {
    // Create an instance of your BurstGraph class
    BurstGraph<false> burstGraph; // Use 'true' for directed graph or 'false' for undirected

    // Load the graph from a file (replace 'input_file.txt' with your file path)
    // if (!burstGraph.LoadGraphFromFile("/data/yuhang/Rush_experiment/bayc/static.txt")) {
    if (!burstGraph.LoadGraphFromFile("/home/yuhang/Work/BurstingGraphMining/test3.txt")) {
        std::cerr << "Failed to load the graph from the file." << std::endl;
        return 1;
    }



    burstGraph.GeneratePeelingSequence();

    burstGraph.FindDenseSubgraph();

    burstGraph.OutputDenseSubgraph("Dense_subgraph.txt");


    /*
    // Access the peeling sequence (assuming 'peeling_sequence' is public)
    const std::vector<std::pair<std::string, double>>& peelingSequence = burstGraph.peeling_sequence;

    // Print the peeling sequence if needed
    for (const auto& node : peelingSequence) {
        std::cout << "Node ID: " << node.first << ", Peeling Weight: " << node.second << std::endl;
    }
    */

    return 0;
}
