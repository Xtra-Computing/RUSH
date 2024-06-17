#include "burst_graph.hpp"
#include <chrono> // For measuring time

template class BurstGraph<true>;
template class BurstGraph<false>;

struct CompareNodeByWeight {
    bool operator()(const std::pair<std::string, double>& left, const std::pair<std::string, double>& right) const {
        if (left.second != right.second) {
            return left.second < right.second;
        }
        return left.first < right.first;  // Use ID for tie-breaking
    }
};

// Implementations of BurstGraph class functions

template <bool Directed>
BurstGraph<Directed>::BurstGraph() {
    // Initialize the graph with custom properties
    g = BurstGraphType();
}

template <bool Directed>
bool BurstGraph<Directed>::LoadGraphFromFile(const std::string& filename, int flag) {
    std::ifstream input_file(filename);
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open the input file." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(input_file, line)) {
        std::istringstream iss(line);
        std::string source_id, dest_id;
        double weight;
        int timestamp;

        if (!(iss >> source_id >> dest_id >> weight >> timestamp)) {
            std::cerr << "Error: Invalid input line format." << std::endl;
            return false;
        }

        if(flag >= 1){
            if(timestamp < this->time_slice_settings.start_time){
                continue;
            }
        }

        if(flag >=2 && weight < 0.1){
            continue;
        }

        // Add vertices and edges to the graph based on the input data
        // Get or add source vertex
        typename BurstGraphType::vertex_descriptor source_vertex;
        if (id_map.find(source_id) == id_map.end()) {
            source_vertex = boost::add_vertex({source_id, 0.0}, g);  // Set weight to 0.0 for now
            id_map[source_id] = source_vertex;
        } else {
            source_vertex = id_map[source_id];
        }

        // Get or add destination vertex
        typename BurstGraphType::vertex_descriptor dest_vertex;
        if (id_map.find(dest_id) == id_map.end()) {
            dest_vertex = boost::add_vertex({dest_id, 0.0}, g);  // Set weight to 0.0 for now
            id_map[dest_id] = dest_vertex;
        } else {
            dest_vertex = id_map[dest_id];
        }

        boost::add_edge(source_vertex, dest_vertex, {weight, timestamp}, g);
    }
    BuildWeightMap();
    BuildEdgeTimeIndex();

    input_file.close();
    return true;
}

template <bool Directed>
const typename BurstGraph<Directed>::BurstGraphType& BurstGraph<Directed>::GetGraph() const {
    return g;
}

template <bool Directed>
void BurstGraph<Directed>::BuildIdMap() {
    id_map.clear();
    typename boost::graph_traits<BurstGraphType>::vertex_iterator vIt, vEnd;

    for (boost::tie(vIt, vEnd) = boost::vertices(g); vIt != vEnd; ++vIt) {
        const std::string& vertex_id = g[*vIt].id;
        id_map[vertex_id] = *vIt;
    }
}

// Implementation of BuildWeightMap (inside the class)
template <bool Directed>
void BurstGraph<Directed>::BuildWeightMap(WeightFunctionType type) {
    // Clear the existing weight_map
    weight_map.clear();

    // Iterate through the vertices and calculate the weight for each node
    for (typename BurstGraphType::vertex_descriptor vd : boost::make_iterator_range(boost::vertices(g))) {
        const std::string& node_id = g[vd].id;
        double weight = WeightFunction(node_id, type);
        
        // Create a pair for the node
        std::pair<std::string, double> node_pair = {node_id, weight};
        
        // Insert the node into the weight_map
        weight_map.insert(node_pair);

        id_weight_map[node_id] = weight;  // Update id_weight_map
    }
}

// Implementation of WeightFunction (outside the class)
template <bool Directed>
double BurstGraph<Directed>::WeightFunction(const std::string& node_id, WeightFunctionType function_type) {
    typename BurstGraphType::vertex_descriptor vd = id_map[node_id];

    switch (function_type) {
        case WeightFunctionType::TotalWeight:
            return CalculateTotalWeight(vd);
        case WeightFunctionType::Degree:
            return CalculateDegree(vd);
        default:
            // Default behavior, you can handle it as needed
            return 0.0;
    }
}

// Implementation of CalculateTotalWeight (outside the class)
template <bool Directed>
double BurstGraph<Directed>::CalculateTotalWeight(const typename BurstGraphType::vertex_descriptor& vd) {
    double total_weight = 0.0;

    for (typename BurstGraphType::out_edge_iterator edge_it = boost::out_edges(vd, g).first;
                edge_it != boost::out_edges(vd, g).second; ++edge_it) {
        total_weight += g[*edge_it].weight;
    }
    return total_weight;
}

template <bool Directed>
void BurstGraph<Directed>::UpdateEdgeWeights(int current_key) {
    std::set<TimeSliceKey> keys_to_remove;
    std::set<TimeSliceKey> keys_to_halve;

    // Identify keys for removal and halving
    for (const auto& time_slice_pair : edge_time_index) {
        TimeSliceKey time_slice_key = time_slice_pair.first;

        if (time_slice_key <= current_key - time_slice_settings.half_life_times * time_slice_settings.retirement_times) {
            // Mark key for removal
            keys_to_remove.insert(time_slice_key);
        } else {
            // Check for halving the weight
            int difference = current_key - time_slice_key;
            if (difference % time_slice_settings.half_life_times == 0) {
                // Mark key for halving
                keys_to_halve.insert(time_slice_key);
                break; // Break the loop once a match is found
            }
        }
    }

    // Remove edges for keys marked for removal
    for (auto key : keys_to_remove) {
        EdgeList& edges = edge_time_index[key];
        for (auto edge : edges) {
            boost::remove_edge(edge, g);
        }
        edge_time_index.erase(key); // Remove the key from the index
    }

    // Halve the weight of edges for keys marked for halving
    for (auto key : keys_to_halve) {
        EdgeList& edges = edge_time_index[key];
        for (auto edge : edges) {
            g[edge].weight /= 2;
        }
    }
    this->BuildWeightMap();
}


// Implementation of CalculateDegree (outside the class)
template <bool Directed>
int BurstGraph<Directed>::CalculateDegree(const typename BurstGraphType::vertex_descriptor& vd) {
    return boost::out_degree(vd, g);
}

// Implementation of GeneratePeelingSequence (outside the class)
template <bool Directed>
void BurstGraph<Directed>::GeneratePeelingSequence() {
    peeling_sequence.clear();

    while (!weight_map.empty()) {
        // Get the node with minimum weight from the set (begin() points to the minimum element)
        std::pair<std::string, double> min_node = *weight_map.begin();
        
        // Remove the min_node from the set
        weight_map.erase(weight_map.begin());

        // Add the min_node to the peeling sequence
        peeling_sequence.push_back(min_node);

        // Update the weight_map for neighbors of the min_node
        UpdateWeightMap(min_node.first);
    }
}

// Implementation of UpdateWeightMap (outside the class)
template <bool Directed>
void BurstGraph<Directed>::UpdateWeightMap(const std::string& node_id) {
    typename BurstGraphType::vertex_descriptor vd = id_map[node_id];

    // Iterate through the neighbors of the node
    for (typename BurstGraphType::out_edge_iterator edge_it = boost::out_edges(vd, g).first;
                edge_it != boost::out_edges(vd, g).second; ++edge_it) {
        typename BurstGraphType::vertex_descriptor target = boost::target(*edge_it, g);
        const std::string& neighbor_id = g[target].id;
        double neighbor_weight = id_weight_map[neighbor_id];

        // Create a pair for the neighbor node
        std::pair<std::string, double> neighbor_pair = {neighbor_id, neighbor_weight};

        // Check if the neighbor is in the set
        auto neighbor_it = weight_map.find(neighbor_pair);
        if (neighbor_it != weight_map.end()) {
            neighbor_weight -= g[*edge_it].weight;

            id_weight_map[neighbor_id] = neighbor_weight;

            // Remove the neighbor from the set
            weight_map.erase(neighbor_it);

            // Update the neighbor's weight and add it back to the set
            neighbor_pair.second = neighbor_weight;
            weight_map.insert(neighbor_pair);
        }
    }
}


template <bool Directed>
void BurstGraph<Directed>::FindDenseSubgraph() {
    max_dense_metric = 0;
    double sum_weights = 0.0;
    int num_nodes = 0;

    // Start from the last node in the peeling sequence and work backward
    for (int i = peeling_sequence.size() - 1; i >= 0; --i) {
        const auto& node = peeling_sequence[i];
        
        // Important: This line may vary for user-defined dense function
        sum_weights += node.second;
        num_nodes++;

        double currentDenseMetric = sum_weights / num_nodes;
        if (currentDenseMetric >= max_dense_metric) {
            max_dense_metric = currentDenseMetric;
            max_index = i; // Update max_index with the current position in the peeling sequence
        }
    }

    // std::cout << "Num of nodes in dense subgraph: " << peeling_sequence.size() - max_index << std::endl;
    // std::cout << "Max metric: " << max_dense_metric << std::endl;
}

// Function to output the dense subgraph in the specified format
template <bool Directed>
void BurstGraph<Directed>::OutputDenseSubgraph(const std::string& outputFilename) {
    std::ofstream output_file(outputFilename, std::ios::app);
    if (!output_file.is_open()) {
        std::cerr << "Error: Unable to open the output file." << std::endl;
        return;
    }

    // Create a set to store the IDs of nodes in the dense subgraph
    std::set<std::string> dense_subgraph_nodes;

    // Populate the set with node IDs from the dense subgraph
    for (int i = max_index; i < peeling_sequence.size(); ++i) {
        dense_subgraph_nodes.insert(peeling_sequence[i].first);
    }

    for(auto & node : dense_subgraph_nodes){
        output_file << node << std::endl;
    }

    output_file.close(); // Close the file after writing the edges
}

// // Function to output the dense subgraph in the specified format
// template <bool Directed>
// void BurstGraph<Directed>::OutputDenseSubgraph(const std::string& outputFilename) {
//     std::ofstream output_file(outputFilename, std::ios::app);
//     if (!output_file.is_open()) {
//         std::cerr << "Error: Unable to open the output file." << std::endl;
//         return;
//     }

//     // Create a set to store the IDs of nodes in the dense subgraph
//     std::set<std::string> dense_subgraph_nodes;

//     // Populate the set with node IDs from the dense subgraph
//     for (int i = max_index; i < peeling_sequence.size(); ++i) {
//         dense_subgraph_nodes.insert(peeling_sequence[i].first);
//     }

//     // Initialize the edge count
//     int edge_count = 0;

//     // Iterate through the dense subgraph nodes and their neighbors
//     for (const auto& node_id : dense_subgraph_nodes) {
//         typename BurstGraphType::vertex_descriptor vd = id_map[node_id];

//         for (const auto& neighbor_vd : boost::make_iterator_range(boost::adjacent_vertices(vd, g))) {
//             const std::string& neighbor_id = g[neighbor_vd].id;

//             // Check if the neighbor is also in the dense subgraph
//             if (dense_subgraph_nodes.count(neighbor_id)) {
//                 if((Directed == false ) && (neighbor_id < node_id)){
//                     continue;
//                 }
//                 // Output the edge in the specified format
//                 output_file << node_id << " " << neighbor_id << " " << g[boost::edge(vd, neighbor_vd, g).first].weight << std::endl;
//                 edge_count++;
//             }
//         }
//     }

//     output_file.close(); // Close the file after writing the edges

//     // Now, let's prepend the vertices and edges count
//     std::ifstream input_file(outputFilename);
//     std::string file_content((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

//     output_file.open(outputFilename); // Open the file for writing again
//     output_file << dense_subgraph_nodes.size() << " " << edge_count << std::endl << file_content;

//     output_file.close();
//     input_file.close();
// }

template <bool Directed>
typename BurstGraph<Directed>::TimeSliceKey BurstGraph<Directed>::GenerateTimeSliceKey(int timestamp) {
    return (timestamp - time_slice_settings.start_time) / time_slice_settings.time_duration;
}


template <bool Directed>
void BurstGraph<Directed>::BuildEdgeTimeIndex() {
    edge_time_index.clear();
    typename boost::graph_traits<BurstGraphType>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
        int timestamp = g[*ei].timestamp;
        TimeSliceKey key = GenerateTimeSliceKey(timestamp);
        edge_time_index[key].push_back(*ei);
    }
}

template <bool Directed>
void BurstGraph<Directed>::SetTimeSliceSettings(int start_time, int duration, int time1, int time2) {
    time_slice_settings.time_duration = duration;
    time_slice_settings.start_time = start_time;
    time_slice_settings.half_life_times = time1;
    time_slice_settings.retirement_times = time2;
}


template <bool Directed>
void BurstGraph<Directed>::AddOneEdge(const std::string& source_vertex, const std::string& target_vertex, double weight, int timestamp, int flag) {
    // Check if the source vertex exists, if not, add it
    typename BurstGraphType::vertex_descriptor source_vertex_desc;
    if(id_map.find(source_vertex) == id_map.end()) {
        source_vertex_desc = boost::add_vertex({source_vertex, 0.0}, g);
        id_map[source_vertex] = source_vertex_desc;
    } else {
        source_vertex_desc = id_map[source_vertex];
    }

    // Check if the target vertex exists, if not, add it
    typename BurstGraphType::vertex_descriptor target_vertex_desc;
    if(id_map.find(target_vertex) == id_map.end()) {
        target_vertex_desc = boost::add_vertex({target_vertex, 0.0}, g);
        id_map[target_vertex] = target_vertex_desc;
    } else {
        target_vertex_desc = id_map[target_vertex];
    }

    // Add the edge
    typename BurstGraphType::edge_descriptor new_edge = boost::add_edge(source_vertex_desc, target_vertex_desc, {weight, timestamp}, g).first;

    // Update the edge_time_index for this particular edge
    TimeSliceKey current_time_slice = GenerateTimeSliceKey(timestamp);
    edge_time_index[current_time_slice].push_back(new_edge);

    if(flag){
        UpdatePeelingSequence(source_vertex, target_vertex, weight);
    }
}



template <bool Directed>
std::set<std::string> BurstGraph<Directed>::FindKCore(int k) {
    std::set<std::string> k_core_nodes;

    // Create a copy of the graph, because the k-core algorithm modifies the graph
    BurstGraphType g_copy = g;

    // Compute the core numbers for each vertex
    std::map<typename BurstGraphType::vertex_descriptor, int> core_numbers;
    boost::associative_property_map< std::map<typename BurstGraphType::vertex_descriptor, int> > core_number_map(core_numbers);
    boost::core_numbers(g_copy, core_number_map);

    // Iterate through the vertices and check their core numbers
    typename boost::graph_traits<BurstGraphType>::vertex_iterator vIt, vEnd;
    for (boost::tie(vIt, vEnd) = boost::vertices(g); vIt != vEnd; ++vIt) {
        if (core_numbers[*vIt] >= k) {
            k_core_nodes.insert(g[*vIt].id);
        }
    }

    return k_core_nodes;
}

template <bool Directed>
void BurstGraph<Directed>::UpdatePeelingSequence(const std::string& source_vertex, const std::string& target_vertex, double new_edge_weight) {
    //TODO: Delete this
    int count = 0;
    int count2 = 0;

    // Find the indices of source and target vertices in the peeling sequence
    int source_index = -1;
    int target_index = -1;



    for (size_t i = 0; i < peeling_sequence.size(); ++i) {
        if (peeling_sequence[i].first == source_vertex) {
            source_index = i;
        }
        if (peeling_sequence[i].first == target_vertex) {
            target_index = i;
        }
        // If both indices are found, break out of the loop
        if (source_index != -1 && target_index != -1) {
            break;
        }
    }

        // auto start = std::chrono::high_resolution_clock::now();



    // std::cout << source_index << " " << target_index << std::endl;

    // Find the smaller index of the two
    int smaller_index = (source_index < target_index) ? source_index : target_index;
    int bigger_index = (source_index > target_index) ? source_index : target_index;


    if(bigger_index != -1 && (peeling_sequence[bigger_index].second + new_edge_weight < max_dense_metric)){
        return;
    }

    // Construct the updated peeling sequence excluding the node at smaller_index
    std::vector<std::pair<std::string, double>> updated_peeling_sequence;
    for (int i = 0; i < smaller_index; ++i) {
        updated_peeling_sequence.push_back(peeling_sequence[i]);
    }

    // Initialize a pending sequence set with custom comparator
    std::set<std::pair<std::string, double>, CompareNodeByWeight> pending_sequence;

    std::map<std::string, int> pending_neighbor_count;

    // Add the node at smaller_index to the pending sequence
    // and increment its weight with the new edge's weight
    if(smaller_index != -1){
        std::string node_id_at_smaller_index = peeling_sequence[smaller_index].first;
        double updated_weight = peeling_sequence[smaller_index].second + new_edge_weight;
        pending_sequence.insert({node_id_at_smaller_index, updated_weight});
        
        // Increase the count for all neighbors of this neighbor in pending_neighbor_count
        typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
        for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[node_id_at_smaller_index], g); out_j != out_end_j; ++out_j) {
            typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
            const std::string& neighbor_id_j = g[neighbor_vd_j].id;
            pending_neighbor_count[neighbor_id_j]++;
        }

    }
    // else{
    //     if(source_index == -1){
    //         auto it = std::find_if(peeling_sequence.begin(), peeling_sequence.end(),
    //                    [new_edge_weight](const std::pair<std::string, double>& element) {
    //                        return element.second > new_edge_weight;
    //                    });

    //         peeling_sequence.insert(it, {source_vertex, new_edge_weight});
    //     }

    //     if(target_index == -1){
    //         auto it = std::find_if(peeling_sequence.begin(), peeling_sequence.end(),
    //                    [new_edge_weight](const std::pair<std::string, double>& element) {
    //                        return element.second > new_edge_weight;
    //                    });

    //         peeling_sequence.insert(it, {target_vertex, new_edge_weight});
    //     }
    //     return;
    // }

    else{
        if(source_index == -1){
            id_weight_map[source_vertex] = new_edge_weight;
            pending_sequence.insert({source_vertex, new_edge_weight});
            // Increase the count for all neighbors of this neighbor in pending_neighbor_count
            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
            for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[source_vertex], g); out_j != out_end_j; ++out_j) {
                typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
                const std::string& neighbor_id_j = g[neighbor_vd_j].id;
                pending_neighbor_count[neighbor_id_j]++;
            }
        }

        if(target_index == -1){
            id_weight_map[target_vertex] = new_edge_weight;
            pending_sequence.insert({target_vertex, new_edge_weight});
            // Increase the count for all neighbors of this neighbor in pending_neighbor_count
            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
            for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[target_vertex], g); out_j != out_end_j; ++out_j) {
                typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
                const std::string& neighbor_id_j = g[neighbor_vd_j].id;
                pending_neighbor_count[neighbor_id_j]++;
            }
        }
    }



    for (int i = smaller_index + 1; i < peeling_sequence.size(); ++i) {
        std::pair<std::string, double>& current_node = peeling_sequence[i];

        // If the current node's weight is equal to or bigger than the first node in the pending sequence
        while (!pending_sequence.empty() && current_node.second >= pending_sequence.begin()->second) {
            // Pop the first node from pending sequence
            std::pair<std::string, double> popped_node = *pending_sequence.begin();
            pending_sequence.erase(pending_sequence.begin());

            id_weight_map[popped_node.first] = popped_node.second;

            // Add the popped node to the updated peeling sequence
            updated_peeling_sequence.push_back(popped_node);

            // Check the neighbors of the popped node
            typename BurstGraphType::vertex_descriptor vd = id_map[popped_node.first];

            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_i, out_end;
            for (boost::tie(out_i, out_end) = boost::out_edges(vd, g); out_i != out_end; ++out_i) {
                typename BurstGraphType::vertex_descriptor neighbor_vd = boost::target(*out_i, g);
                const std::string& neighbor_id = g[neighbor_vd].id;

                // Decrease the count for this neighbor in pending_neighbor_count
                pending_neighbor_count[neighbor_id]--;


                // Find this neighbor in the pending sequence
                auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
                if (it != pending_sequence.end()) {
                    // Reduce the neighbor's weight by the weight of the edge
                    std::pair<std::string, double> new_neighbor = {neighbor_id, it->second - g[*out_i].weight};
                    pending_sequence.erase(it);
                    pending_sequence.insert(new_neighbor);
                }
            }


        }

        // If the current node's weight is smaller than the first node in the pending sequence
        if (!pending_sequence.empty() && current_node.second < (*pending_sequence.begin()).second) {
            count += 1;
            typename BurstGraphType::vertex_descriptor vd_current = id_map[current_node.first];

            if (pending_neighbor_count.find(current_node.first) == pending_neighbor_count.end()){
            }
            else if(pending_neighbor_count[current_node.first] == 0){
            }
            else{
                count2++;
                // Check the neighbors of the current node
                for (const auto& neighbor_vd : boost::make_iterator_range(boost::adjacent_vertices(vd_current, g))) {
                    const std::string& neighbor_id = g[neighbor_vd].id;

                    // Find this neighbor in the pending sequence
                    auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
                    if (it != pending_sequence.end()) {
                        typename BurstGraphType::edge_descriptor ed = boost::edge(vd_current, neighbor_vd, g).first;

                        // Update the current node's weight
                        current_node.second += g[ed].weight;

                        // Update the neighbor's peeling weight in the pending sequence
                        std::pair<std::string, double> new_neighbor = {neighbor_id, it->second + g[ed].weight};
                        pending_sequence.erase(it);
                        pending_sequence.insert(new_neighbor);

                        // Update the id_weight_map for the neighbor
                        id_weight_map[neighbor_id] = new_neighbor.second;
                    }
                }
            }

            // Add the current node to the pending sequence after updating its weight
            pending_sequence.insert(current_node);
            id_weight_map[current_node.first] = current_node.second;

            // Increase the count for all neighbors of this neighbor in pending_neighbor_count
            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
            for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[current_node.first], g); out_j != out_end_j; ++out_j) {
                typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
                const std::string& neighbor_id_j = g[neighbor_vd_j].id;
                pending_neighbor_count[neighbor_id_j]++;
            }

        }

        // If pending sequence is empty, add the rest of the nodes from old sequence
        if (pending_sequence.empty()) {
            for (int j = i; j < peeling_sequence.size(); ++j) {
                updated_peeling_sequence.push_back(peeling_sequence[j]);
            }
            break;  // Break from the main loop since we've added all nodes
        }

    }

    while (!pending_sequence.empty()) {
        // Pop the first node from pending sequence
        std::pair<std::string, double> popped_node = *pending_sequence.begin();
        pending_sequence.erase(pending_sequence.begin());

        id_weight_map[popped_node.first] = popped_node.second;

        // Add the popped node to the updated peeling sequence
        updated_peeling_sequence.push_back(popped_node);

        // Check the neighbors of the popped node
        typename BurstGraphType::vertex_descriptor vd = id_map[popped_node.first];

        typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_i, out_end;
        for (boost::tie(out_i, out_end) = boost::out_edges(vd, g); out_i != out_end; ++out_i) {
            typename BurstGraphType::vertex_descriptor neighbor_vd = boost::target(*out_i, g);
            const std::string& neighbor_id = g[neighbor_vd].id;

            // Decrease the count for this neighbor in pending_neighbor_count
            pending_neighbor_count[neighbor_id]--;


            // Find this neighbor in the pending sequence
            auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
            if (it != pending_sequence.end()) {
                // Reduce the neighbor's weight by the weight of the edge
                std::pair<std::string, double> new_neighbor = {neighbor_id, it->second - g[*out_i].weight};
                pending_sequence.erase(it);
                pending_sequence.insert(new_neighbor);
            }
        }


    }

    peeling_sequence = updated_peeling_sequence;
}


// template <bool Directed>
// int BurstGraph<Directed>::BatchUpdatePeelingSequenceTest(int key){
//     EdgeList& edges = edge_time_index[key];
//         // for (auto edge : edges) {
//             // g[edge].weight /= 2;
//         // }

//     int count = 0;
//     int count2 = 0;

//     int first_index = -1;
//     for(size_t i = 0; i < peeling_sequence.size(); ++i){
//         for(auto edge: edges){
//             auto source = boost::source(edge, g);
//             auto target = boost::target(edge, g);
//             if(source == peeling_sequence[i].first){
//                 first_index = source;
//                 break;
//             }
//             if(target == peeling_sequence[i].first){
//                 first_index = target;
//                 break;
//         }

//         if(first_index != -1){
//             break;
//         }
//     }

//         // auto start = std::chrono::high_resolution_clock::now();



//     // std::cout << source_index << " " << target_index << std::endl;

//     // Find the smaller index of the two
//     int smaller_index = (source_index < target_index) ? source_index : target_index;
//     int bigger_index = (source_index > target_index) ? source_index : target_index;


//     if(bigger_index != -1 && (peeling_sequence[bigger_index].second + new_edge_weight < max_dense_metric)){
//         return;
//     }

//     // Construct the updated peeling sequence excluding the node at smaller_index
//     std::vector<std::pair<std::string, double>> updated_peeling_sequence;
//     for (int i = 0; i < smaller_index; ++i) {
//         updated_peeling_sequence.push_back(peeling_sequence[i]);
//     }

//     // Initialize a pending sequence set with custom comparator
//     std::set<std::pair<std::string, double>, CompareNodeByWeight> pending_sequence;

//     std::map<std::string, int> pending_neighbor_count;

//     // Add the node at smaller_index to the pending sequence
//     // and increment its weight with the new edge's weight
//     if(smaller_index != -1){
//         std::string node_id_at_smaller_index = peeling_sequence[smaller_index].first;
//         double updated_weight = peeling_sequence[smaller_index].second + new_edge_weight;
//         pending_sequence.insert({node_id_at_smaller_index, updated_weight});
        
//         // Increase the count for all neighbors of this neighbor in pending_neighbor_count
//         typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
//         for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[node_id_at_smaller_index], g); out_j != out_end_j; ++out_j) {
//             typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
//             const std::string& neighbor_id_j = g[neighbor_vd_j].id;
//             pending_neighbor_count[neighbor_id_j]++;
//         }

//     }
//     else{
//         if(source_index == -1){
//             id_weight_map[source_vertex] = new_edge_weight;
//             pending_sequence.insert({source_vertex, new_edge_weight});
//             // Increase the count for all neighbors of this neighbor in pending_neighbor_count
//             typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
//             for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[source_vertex], g); out_j != out_end_j; ++out_j) {
//                 typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
//                 const std::string& neighbor_id_j = g[neighbor_vd_j].id;
//                 pending_neighbor_count[neighbor_id_j]++;
//             }
//         }

//         if(target_index == -1){
//             id_weight_map[target_vertex] = new_edge_weight;
//             pending_sequence.insert({target_vertex, new_edge_weight});
//             // Increase the count for all neighbors of this neighbor in pending_neighbor_count
//             typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
//             for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[target_vertex], g); out_j != out_end_j; ++out_j) {
//                 typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
//                 const std::string& neighbor_id_j = g[neighbor_vd_j].id;
//                 pending_neighbor_count[neighbor_id_j]++;
//             }
//         }
//     }



//     for (int i = smaller_index + 1; i < peeling_sequence.size(); ++i) {
//         std::pair<std::string, double>& current_node = peeling_sequence[i];

//         // If the current node's weight is equal to or bigger than the first node in the pending sequence
//         while (!pending_sequence.empty() && current_node.second >= pending_sequence.begin()->second) {
//             // Pop the first node from pending sequence
//             std::pair<std::string, double> popped_node = *pending_sequence.begin();
//             pending_sequence.erase(pending_sequence.begin());

//             id_weight_map[popped_node.first] = popped_node.second;

//             // Add the popped node to the updated peeling sequence
//             updated_peeling_sequence.push_back(popped_node);

//             // Check the neighbors of the popped node
//             typename BurstGraphType::vertex_descriptor vd = id_map[popped_node.first];

//             typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_i, out_end;
//             for (boost::tie(out_i, out_end) = boost::out_edges(vd, g); out_i != out_end; ++out_i) {
//                 typename BurstGraphType::vertex_descriptor neighbor_vd = boost::target(*out_i, g);
//                 const std::string& neighbor_id = g[neighbor_vd].id;

//                 // Decrease the count for this neighbor in pending_neighbor_count
//                 pending_neighbor_count[neighbor_id]--;


//                 // Find this neighbor in the pending sequence
//                 auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
//                 if (it != pending_sequence.end()) {
//                     // Reduce the neighbor's weight by the weight of the edge
//                     std::pair<std::string, double> new_neighbor = {neighbor_id, it->second - g[*out_i].weight};
//                     pending_sequence.erase(it);
//                     pending_sequence.insert(new_neighbor);
//                 }
//             }


//         }

//         // If the current node's weight is smaller than the first node in the pending sequence
//         if (!pending_sequence.empty() && current_node.second < (*pending_sequence.begin()).second) {
//             count += 1;
//             typename BurstGraphType::vertex_descriptor vd_current = id_map[current_node.first];

//             if (pending_neighbor_count.find(current_node.first) == pending_neighbor_count.end()){
//             }
//             else if(pending_neighbor_count[current_node.first] == 0){
//             }
//             else{
//                 count2++;
//                 // Check the neighbors of the current node
//                 for (const auto& neighbor_vd : boost::make_iterator_range(boost::adjacent_vertices(vd_current, g))) {
//                     const std::string& neighbor_id = g[neighbor_vd].id;

//                     // Find this neighbor in the pending sequence
//                     auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
//                     if (it != pending_sequence.end()) {
//                         typename BurstGraphType::edge_descriptor ed = boost::edge(vd_current, neighbor_vd, g).first;

//                         // Update the current node's weight
//                         current_node.second += g[ed].weight;

//                         // Update the neighbor's peeling weight in the pending sequence
//                         std::pair<std::string, double> new_neighbor = {neighbor_id, it->second + g[ed].weight};
//                         pending_sequence.erase(it);
//                         pending_sequence.insert(new_neighbor);

//                         // Update the id_weight_map for the neighbor
//                         id_weight_map[neighbor_id] = new_neighbor.second;
//                     }
//                 }
//             }

//             // Add the current node to the pending sequence after updating its weight
//             pending_sequence.insert(current_node);
//             id_weight_map[current_node.first] = current_node.second;

//             // Increase the count for all neighbors of this neighbor in pending_neighbor_count
//             typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
//             for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[current_node.first], g); out_j != out_end_j; ++out_j) {
//                 typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
//                 const std::string& neighbor_id_j = g[neighbor_vd_j].id;
//                 pending_neighbor_count[neighbor_id_j]++;
//             }

//         }

//         // If pending sequence is empty, add the rest of the nodes from old sequence
//         if (pending_sequence.empty()) {
//             for (int j = i; j < peeling_sequence.size(); ++j) {
//                 updated_peeling_sequence.push_back(peeling_sequence[j]);
//             }
//             break;  // Break from the main loop since we've added all nodes
//         }

//     }

//     while (!pending_sequence.empty()) {
//         // Pop the first node from pending sequence
//         std::pair<std::string, double> popped_node = *pending_sequence.begin();
//         pending_sequence.erase(pending_sequence.begin());

//         id_weight_map[popped_node.first] = popped_node.second;

//         // Add the popped node to the updated peeling sequence
//         updated_peeling_sequence.push_back(popped_node);

//         // Check the neighbors of the popped node
//         typename BurstGraphType::vertex_descriptor vd = id_map[popped_node.first];

//         typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_i, out_end;
//         for (boost::tie(out_i, out_end) = boost::out_edges(vd, g); out_i != out_end; ++out_i) {
//             typename BurstGraphType::vertex_descriptor neighbor_vd = boost::target(*out_i, g);
//             const std::string& neighbor_id = g[neighbor_vd].id;

//             // Decrease the count for this neighbor in pending_neighbor_count
//             pending_neighbor_count[neighbor_id]--;


//             // Find this neighbor in the pending sequence
//             auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
//             if (it != pending_sequence.end()) {
//                 // Reduce the neighbor's weight by the weight of the edge
//                 std::pair<std::string, double> new_neighbor = {neighbor_id, it->second - g[*out_i].weight};
//                 pending_sequence.erase(it);
//                 pending_sequence.insert(new_neighbor);
//             }
//         }


//     }

//     peeling_sequence = updated_peeling_sequence;
//     return count;
// }

/*
template <bool Directed>
int BurstGraph<Directed>::UpdatePeelingSequenceTest(const std::string& source_vertex, const std::string& target_vertex, double new_edge_weight) {
    int count = 0;

    // Find the indices of source and target vertices in the peeling sequence
    int source_index = -1;
    int target_index = -1;

    for (size_t i = 0; i < peeling_sequence.size(); ++i) {
        if (peeling_sequence[i].first == source_vertex) {
            source_index = i;
        }
        if (peeling_sequence[i].first == target_vertex) {
            target_index = i;
        }
        // If both indices are found, break out of the loop
        if (source_index != -1 && target_index != -1) {
            break;
        }
    }

    // Find the smaller index of the two
    int smaller_index = (source_index < target_index) ? source_index : target_index;
    int bigger_index = (source_index > target_index) ? source_index : target_index;


    if(bigger_index != -1 && (peeling_sequence[bigger_index].second + new_edge_weight < max_dense_metric)){
        return;
    }

    // Construct the updated peeling sequence excluding the node at smaller_index
    std::vector<std::pair<std::string, double>> updated_peeling_sequence;
    for (int i = 0; i < smaller_index; ++i) {
        updated_peeling_sequence.push_back(peeling_sequence[i]);
    }

    // Initialize a pending sequence set with custom comparator
    std::set<std::pair<std::string, double>, CompareNodeByWeight> pending_sequence;

    std::map<std::string, int> pending_neighbor_count;

    // Add the node at smaller_index to the pending sequence
    // and increment its weight with the new edge's weight
    if(smaller_index != -1){
        std::string node_id_at_smaller_index = peeling_sequence[smaller_index].first;
        double updated_weight = peeling_sequence[smaller_index].second + new_edge_weight;
        pending_sequence.insert({node_id_at_smaller_index, updated_weight});
        
        // Increase the count for all neighbors of this neighbor in pending_neighbor_count
        typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
        for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[node_id_at_smaller_index], g); out_j != out_end_j; ++out_j) {
            typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
            const std::string& neighbor_id_j = g[neighbor_vd_j].id;
            pending_neighbor_count[neighbor_id_j]++;
        }

    }
    else{
        if(source_index == -1){
            id_weight_map[source_vertex] = new_edge_weight;
            pending_sequence.insert({source_vertex, new_edge_weight});
            // Increase the count for all neighbors of this neighbor in pending_neighbor_count
            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
            for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[source_vertex], g); out_j != out_end_j; ++out_j) {
                typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
                const std::string& neighbor_id_j = g[neighbor_vd_j].id;
                pending_neighbor_count[neighbor_id_j]++;
            }
        }

        if(target_index == -1){
            id_weight_map[target_vertex] = new_edge_weight;
            pending_sequence.insert({target_vertex, new_edge_weight});
            // Increase the count for all neighbors of this neighbor in pending_neighbor_count
            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
            for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[target_vertex], g); out_j != out_end_j; ++out_j) {
                typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
                const std::string& neighbor_id_j = g[neighbor_vd_j].id;
                pending_neighbor_count[neighbor_id_j]++;
            }
        }
    }



    for (int i = smaller_index + 1; i < peeling_sequence.size(); ++i) {
        std::pair<std::string, double>& current_node = peeling_sequence[i];

        // If the current node's weight is equal to or bigger than the first node in the pending sequence
        while (!pending_sequence.empty() && current_node.second >= pending_sequence.begin()->second) {
            count++;
            // Pop the first node from pending sequence
            std::pair<std::string, double> popped_node = *pending_sequence.begin();
            pending_sequence.erase(pending_sequence.begin());

            id_weight_map[popped_node.first] = popped_node.second;

            // Add the popped node to the updated peeling sequence
            updated_peeling_sequence.push_back(popped_node);

            // Check the neighbors of the popped node
            typename BurstGraphType::vertex_descriptor vd = id_map[popped_node.first];

            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_i, out_end;
            for (boost::tie(out_i, out_end) = boost::out_edges(vd, g); out_i != out_end; ++out_i) {
                typename BurstGraphType::vertex_descriptor neighbor_vd = boost::target(*out_i, g);
                const std::string& neighbor_id = g[neighbor_vd].id;

                // Decrease the count for this neighbor in pending_neighbor_count
                pending_neighbor_count[neighbor_id]--;


                // Find this neighbor in the pending sequence
                auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
                if (it != pending_sequence.end()) {
                    // Reduce the neighbor's weight by the weight of the edge
                    std::pair<std::string, double> new_neighbor = {neighbor_id, it->second - g[*out_i].weight};
                    pending_sequence.erase(it);
                    pending_sequence.insert(new_neighbor);
                }
            }


        }

        // If the current node's weight is smaller than the first node in the pending sequence
        if (!pending_sequence.empty() && current_node.second < (*pending_sequence.begin()).second) {
            typename BurstGraphType::vertex_descriptor vd_current = id_map[current_node.first];

            if (pending_neighbor_count.find(current_node.first) == pending_neighbor_count.end()){
            }
            else if(pending_neighbor_count[current_node.first] == 0){
            }
            else{
                count2++;
                // Check the neighbors of the current node
                for (const auto& neighbor_vd : boost::make_iterator_range(boost::adjacent_vertices(vd_current, g))) {
                    const std::string& neighbor_id = g[neighbor_vd].id;

                    // Find this neighbor in the pending sequence
                    auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
                    if (it != pending_sequence.end()) {
                        typename BurstGraphType::edge_descriptor ed = boost::edge(vd_current, neighbor_vd, g).first;

                        // Update the current node's weight
                        current_node.second += g[ed].weight;

                        // Update the neighbor's peeling weight in the pending sequence
                        std::pair<std::string, double> new_neighbor = {neighbor_id, it->second + g[ed].weight};
                        pending_sequence.erase(it);
                        pending_sequence.insert(new_neighbor);

                        // Update the id_weight_map for the neighbor
                        id_weight_map[neighbor_id] = new_neighbor.second;
                    }
                }
            }

            // Add the current node to the pending sequence after updating its weight
            pending_sequence.insert(current_node);
            id_weight_map[current_node.first] = current_node.second;

            // Increase the count for all neighbors of this neighbor in pending_neighbor_count
            typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_j, out_end_j;
            for (boost::tie(out_j, out_end_j) = boost::out_edges(id_map[current_node.first], g); out_j != out_end_j; ++out_j) {
                typename BurstGraphType::vertex_descriptor neighbor_vd_j = boost::target(*out_j, g);
                const std::string& neighbor_id_j = g[neighbor_vd_j].id;
                pending_neighbor_count[neighbor_id_j]++;
            }

        }

        // If pending sequence is empty, add the rest of the nodes from old sequence
        if (pending_sequence.empty()) {
            for (int j = i; j < peeling_sequence.size(); ++j) {
                updated_peeling_sequence.push_back(peeling_sequence[j]);
            }
            break;  // Break from the main loop since we've added all nodes
        }

    }

    while (!pending_sequence.empty()) {
        // Pop the first node from pending sequence
        std::pair<std::string, double> popped_node = *pending_sequence.begin();
        pending_sequence.erase(pending_sequence.begin());

        id_weight_map[popped_node.first] = popped_node.second;

        // Add the popped node to the updated peeling sequence
        updated_peeling_sequence.push_back(popped_node);

        // Check the neighbors of the popped node
        typename BurstGraphType::vertex_descriptor vd = id_map[popped_node.first];

        typename boost::graph_traits<BurstGraphType>::out_edge_iterator out_i, out_end;
        for (boost::tie(out_i, out_end) = boost::out_edges(vd, g); out_i != out_end; ++out_i) {
            typename BurstGraphType::vertex_descriptor neighbor_vd = boost::target(*out_i, g);
            const std::string& neighbor_id = g[neighbor_vd].id;

            // Decrease the count for this neighbor in pending_neighbor_count
            pending_neighbor_count[neighbor_id]--;


            // Find this neighbor in the pending sequence
            auto it = pending_sequence.find({neighbor_id, id_weight_map[neighbor_id]});
            if (it != pending_sequence.end()) {
                // Reduce the neighbor's weight by the weight of the edge
                std::pair<std::string, double> new_neighbor = {neighbor_id, it->second - g[*out_i].weight};
                pending_sequence.erase(it);
                pending_sequence.insert(new_neighbor);
            }
        }


    }

    peeling_sequence = updated_peeling_sequence;
    return count;
}
*/