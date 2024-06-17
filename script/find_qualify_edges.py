def find_edges_in_subgraph_optimized(subgraph_nodes_file, basic_txt_file, timestamp, threshold):
    # Step 1: Read subgraph nodes into a set
    with open(subgraph_nodes_file, 'r') as file:
        subgraph_nodes = set(file.read().splitlines())

    # Step 2: Define the timestamp range
    start_time = timestamp - threshold
    end_time = timestamp + threshold

    # Initialize a list to hold the qualified edges
    qualified_edges = []

    # Step 3: Process basic.txt with timestamp filtering first
    with open(basic_txt_file, 'r') as file:
        for line in file:
            source, target, weight, edge_timestamp = line.split()
            edge_timestamp = int(edge_timestamp)  # Assuming timestamp is an integer

            if start_time > edge_timestamp:
                continue

            # Break the loop if the edge timestamp is greater than the end time
            if edge_timestamp > end_time:
                break
            
            # Check if the edge is within the timestamp range and involves nodes in the subgraph
            if source in subgraph_nodes and target in subgraph_nodes:
                qualified_edges.append(line.strip())

    return qualified_edges

# Example usage
subgraph_nodes_file = '/data/yuhang/rush/ios/2_4_16/rush/2356.txt'  # Adjust as necessary
basic_txt_file = '/data/yuhang/rush/ios/basic.txt'  # Adjust as necessary
timestamp = 954403910# Example timestamp
threshold = 1000  # Example threshold

qualified_edges = find_edges_in_subgraph_optimized(subgraph_nodes_file, basic_txt_file, timestamp, threshold)

# Print or process the qualified edges
for edge in qualified_edges:
    print(edge)