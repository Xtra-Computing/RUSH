import math

def process_file(file_path):
    # Dictionary to store the count of edges between each node pair
    edge_counts = {}

    # First pass: Read the file to count edges
    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) != 4:
                continue

            source, target, _, _ = parts
            node_pair = (source, target)
            edge_counts[node_pair] = edge_counts.get(node_pair, 0) + 1

    # Second pass: Read the file again and recalculate weights
    with open(file_path, 'r') as file, open('increment.txt', 'w') as outfile:
        for line in file:
            parts = line.strip().split()
            if len(parts) != 4:
                continue

            source, target, _, timestamp = parts
            node_pair = (source, target)
            x = edge_counts[node_pair]
            new_weight = 1 / math.log(5 + x)
            updated_line = f"{source} {target} {new_weight} {timestamp}\n"
            outfile.write(updated_line)

# Replace 'path_to_your_file.txt' with your file's path
file_path = 'original.txt'
process_file(file_path)
