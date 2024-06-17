#ifndef BURST_GRAPH_HPP
#define BURST_GRAPH_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <queue> // Include the priority_queue header
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/core_numbers.hpp>

// Define custom property for vertex
struct VertexProperties {
    std::string id; // Vertex ID as a string
    double weight;
};

// Define custom property for edge
struct EdgeProperties {
    double weight;
    int timestamp;
};

// Custom comparison operator for the weight_map set
struct CompareWeights {
    bool operator()(const std::pair<std::string, double>& left, const std::pair<std::string, double>& right) const {
        if (left.second != right.second) {
            return left.second < right.second;
        }
        // If weights are equal, compare based on node IDs
        return left.first < right.first;
    }
};

// Define the BurstGraph class
template <bool Directed>
class BurstGraph {
public:
    using BurstGraphType = boost::adjacency_list<
        boost::listS,                          // Vertex container type (vector)
        boost::vecS,                          // Out-edge container type (vector)
        typename boost::conditional<Directed, boost::directedS, boost::undirectedS>::type, // Graph type (directed or undirected)
        VertexProperties,                     // Vertex property type
        EdgeProperties                        // Edge property type
    >;

    // Enumeration to specify weight function types
    enum class WeightFunctionType {
        TotalWeight,
        Degree
    };

    BurstGraphType g;  // The BurstGraph instance

    // Use set to keep track of the node with minimum weight
    std::set<std::pair<std::string, double>, CompareWeights> weight_map;

    // The corresponding map for the weight_map to make sure we can quick check the weight for a certain node
    // we need to use this weight to update the weight_map 
    std::map<std::string, double> id_weight_map;  

    // The map for the burst graph, key is the node ID, value is the vertex
    std::map<std::string, typename BurstGraphType::vertex_descriptor> id_map; 

    // Member variable to store the peeling sequence
    std::vector<std::pair<std::string, double>> peeling_sequence;

    // The dense metric for the densest subgraph
    double max_dense_metric;

    double max_index; 


    // Use those to divide edges into different timeslice
    using TimeSliceKey = int; // An integer key representing the time slice
    using EdgeList = std::vector<typename BurstGraphType::edge_descriptor>;

    // User-defined settings for time slices
    struct TimeSliceSettings {
        int start_time;       // Start time for the first time slice
        int time_duration;   // Duration for each time slice
        // The actual half-life duration is half_life_times*time_duration.
        int half_life_times;
        int retirement_times;
    };

    TimeSliceSettings time_slice_settings;

    std::map<TimeSliceKey, EdgeList> edge_time_index;



    // Constructor
    BurstGraph();

    // Load a graph from an input file
    bool LoadGraphFromFile(const std::string& filename, int flag = 0);

    // Accessor function to get the graph
    const BurstGraphType& GetGraph() const;

    // Build the map for quick access to nodes by their IDs
    void BuildIdMap();

    // Function declaration for BuildWeightMap
    void BuildWeightMap(WeightFunctionType type = WeightFunctionType::TotalWeight);

    // Function to calculate the weight of a node based on the chosen weight function
    double WeightFunction(const std::string& node_id, WeightFunctionType function_type = WeightFunctionType::TotalWeight);

    // Function to generate the peeling sequence using a priority queue (min-heap)
    void GeneratePeelingSequence();

    // Function to find the densest subgraph 
    void FindDenseSubgraph();

    // Function to output the densest subgraph 
    void OutputDenseSubgraph(const std::string& outputFilename); 

    void UpdateEdgeWeights(int current_key);


    void BuildEdgeTimeIndex();

    void SetTimeSliceSettings(int start_time, int duration, int times1, int times2);

    void AddOneEdge(const std::string& source_vertex, const std::string& target_vertex, double weight, int timestamp, int flag=1);

    void UpdatePeelingSequence(const std::string& source_vertex, const std::string& target_vertex, double new_edge_weight);
    int UpdatePeelingSequenceTest(const std::string& source_vertex, const std::string& target_vertex, double new_edge_weight);
    int BatchUpdatePeelingSequenceTest(int number);

    // Given a timestamp, we use this function to calcualte which time slice it belongs to
    TimeSliceKey GenerateTimeSliceKey(int timestamp);


    std::set<std::string> FindKCore(int k);

private:
    // Function to calculate the total weight of a node's adjacency edges
    double CalculateTotalWeight(const typename BurstGraphType::vertex_descriptor& vd);

    // Function to calculate the degree of a node
    int CalculateDegree(const typename BurstGraphType::vertex_descriptor& vd);

    // Function to update the weight_map for neighbors of a node
    void UpdateWeightMap(const std::string& node_id);


};


#endif // BURST_GRAPH_HPP
