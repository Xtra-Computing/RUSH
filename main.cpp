#include "burst_graph.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 8) {
        std::cerr << "Usage: " << argv[0] << " <file1> <file2> <start_time> <time_slice_length> <half_life_ratio> <retirement_ratio> <output_directory>" << std::endl;
        return 1;
    }

    std::string file1 = argv[1];
    std::string file2 = argv[2];
    int start_time = std::stoi(argv[3]);
    int time_slice_length = std::stoi(argv[4]);
    int half_life_ratio = std::stoi(argv[5]);
    int retirement_ratio = std::stoi(argv[6]);
    std::string output_directory = argv[7];

    start_time -= time_slice_length*retirement_ratio;

    // std::string file1 = "/data/yuhang/rush/bayc/static.txt";
    // std::string file2 = "/data/yuhang/rush/bayc/increment.txt";
    // int start_time = 1650364600;
    // int time_slice_length = 60;
    // int half_life_ratio = 1;
    // int retirement_ratio = 1;
    // std::string output_directory = "./";
// 
    // start_time -= time_slice_length*retirement_ratio;

    // Replace with 'true' if you want a directed graph
    BurstGraph<false> graph;
    graph.SetTimeSliceSettings(start_time, time_slice_length, half_life_ratio, retirement_ratio);

    // Load initial graph from file1
    if (!graph.LoadGraphFromFile(file1, 2)) {
        std::cerr << "Failed to load initial graph from " << file1 << std::endl;
        return 1;
    }

    // Update graph with additional edges from file2
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
            graph.BuildWeightMap();
            graph.GeneratePeelingSequence();
            graph.FindDenseSubgraph();
        }
        auto start = std::chrono::high_resolution_clock::now();
        graph.AddOneEdge(source_id, dest_id, weight, timestamp);

        // std::cout << "Sequence length: " << graph.peeling_sequence.size() << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        total_time += elapsed.count();

        graph.FindDenseSubgraph();
        // Output the dense graph to a file in the specified directory
        std::string output_file = output_directory + "/" + std::to_string(count) + ".txt";
        std::ofstream out(output_file);
        if (out.is_open()) {
            // Write elapsed time at the first line
            out << "Elapsed time: " << elapsed.count() << " ms\n";
            out.close();
        } else {
            std::cerr << "Failed to open output file: " << output_file << std::endl;
        }
        graph.OutputDenseSubgraph(output_file);  // Assuming this is the function to output the dense graph
        count++;

        // Uncomment if you want to limit the number of processed edges
        // if(count == 300) {
        //     break;
        // }
    }

    return 0;
}
