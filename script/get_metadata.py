def process_graph(file_path):
    unique_nodes = set()
    num_edges = 0

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split(' ')  # Splitting by double space
            if len(parts) != 4:
                continue  # Skip lines that do not have 4 elements

            source, target = parts[0], parts[1]
            unique_nodes.update([source, target])
            num_edges += 1

    num_nodes = len(unique_nodes)
    average_degree = (2 * num_edges) / num_nodes if num_nodes > 0 else 0

    print(f"Number of Nodes: {num_nodes}")
    print(f"Number of Edges: {num_edges}")
    print(f"Average Degree: {average_degree}")

# Replace 'path_to_your_file.txt' with your file path
file_path = 'original.txt'
process_graph(file_path)
