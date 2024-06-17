#include "burst_graph.hpp"
#include <iostream>
#include <string>
#include <chrono>

int main() {
    // Replace with 'true' if you want a directed graph
    BurstGraph<false> graph;
    graph.SetTimeSliceSettings(1650362600 - 3600*16, 3600*24, 1, 16);

    // Load initial graph from file1
    std::string file1 = "/data/yuhang/Rush_experiment/bayc/static.txt"; // Replace with your actual file path
    if (!graph.LoadGraphFromFile(file1)) {
        std::cerr << "Failed to load initial graph from " << file1 << std::endl;
        return 1;
    }

    // Update graph with additional edges from file2
    std::string file2 = "/data/yuhang/Rush_experiment/bayc/increment.txt"; // Replace with your actual file path
    std::ifstream file2_stream(file2);
    if (!file2_stream.is_open()) {
        std::cerr << "Error: Unable to open " << file2 << std::endl;
        return 1;
    }


    std::string source_id, dest_id;
    double weight;
    int timestamp;
    int current_key = 0;
    int incoming_key;
    double total_time = 0.0;
    int count = 0;

    graph.GeneratePeelingSequence();

    while (file2_stream >> source_id >> dest_id >> weight >> timestamp) {
        incoming_key = graph.GenerateTimeSliceKey(timestamp);
        if(current_key < incoming_key){
            while(current_key < incoming_key){
                current_key += 1;
                graph.UpdateEdgeWeights(incoming_key);
            }
            graph.GeneratePeelingSequence();
            graph.FindDenseSubgraph();
        }
        auto start = std::chrono::high_resolution_clock::now();
        graph.AddOneEdge(source_id, dest_id, weight, timestamp);

        // std::cout << "Sequence length: " << graph.peeling_sequence.size() << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        total_time += elapsed.count();
        count++;
        std::cout << count << std::endl;
        graph.FindDenseSubgraph();
        // if(count == 300){
            // break;
        // }
    }

    std::cout << "Processed " << count << " edges." << std::endl;
    std::cout << "Average time per update: " << total_time / count << " ms." << std::endl;

    // Additional operations on the graph can be performed here
    // For example, finding the densest subgraph, etc.

    return 0;
}
