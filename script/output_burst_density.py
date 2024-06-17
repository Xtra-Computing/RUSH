import os

# Function to calculate the average density of a subgraph
def calculate_average_density(subgraph_nodes, neighbor_list):
    edge_count = 0
    for node in subgraph_nodes:
        # Count only edges where both nodes are in the subgraph
        edge_count += len([neighbor for neighbor in neighbor_list.get(node, []) if neighbor in subgraph_nodes])
    # Each edge is counted twice (once for each node), so divide by 2
    edge_count /= 2
    if len(subgraph_nodes) == 0:
        return 0
    average_density = 2 * edge_count / len(subgraph_nodes)
    return average_density

# Build neighbor list from the static file
neighbor_list = {}
with open('/data/yuhang/rush/ios/static.txt', 'r') as static_file:
    for line in static_file:
        parts = line.split()
        if len(parts) == 4:
            source, target = parts[0], parts[1]
            neighbor_list.setdefault(source, []).append(target)
            neighbor_list.setdefault(target, []).append(source)  # For undirected graphs

# Process each subgraph file and calculate the average density
densities = []
directory = "/data/yuhang/rush/ios/2_4_16/rush/"
for filename in sorted(os.listdir(directory)):
    if filename.endswith(".txt") and filename != 'static.txt':
        with open(directory + filename, 'r') as subgraph_file:
            subgraph_nodes = [line.strip() for line in subgraph_file]
            density = calculate_average_density(subgraph_nodes, neighbor_list)
            densities.append((filename, density))

# Write the densities to an output file
with open('densities.txt', 'w') as output_file:
    for filename, density in densities:
        output_file.write(f"{filename}: {density}\n")
