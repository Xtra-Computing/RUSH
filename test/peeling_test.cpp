#include <iostream>
#include <chrono> // For measuring time
#include "burst_graph.hpp" // Include your BurstGraph class definition

int main() {
    // Create an instance of your BurstGraph class
    BurstGraph<false> burstGraph; // Use 'true' for directed graph or 'false' for undirected

    // Load the graph from a file (replace 'input_file.txt' with your file path)
    if (!burstGraph.LoadGraphFromFile("/home/yuhang/grab_data/burst_test/ios_data_first.txt", 1)) {
        std::cerr << "Failed to load the graph from the file." << std::endl;
        return 1;
    }

    std::cout << "Load Graph Successfully" << std::endl;

    // Start measuring time
    auto startTime = std::chrono::high_resolution_clock::now();

    // Generate the peeling sequence
    burstGraph.GeneratePeelingSequence();

    // Stop measuring time
    auto endTime = std::chrono::high_resolution_clock::now();

    // Calculate and print the elapsed time
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "Peeling time: " << duration.count() << " milliseconds" << std::endl;

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
