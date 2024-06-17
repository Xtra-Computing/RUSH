import os

# Assuming directory_path and fraud_file_path are correctly set
directory_path = "/data/yuhang/rush/ios/2_4_16/rush"  # Update this to your actual path
fraud_file_path = "/data/yuhang/rush/ios/fraud.txt"  # Update this to your actual path

# Load fraud nodes
with open(fraud_file_path, 'r') as fraud_file:
    fraud_nodes = set(line.strip() for line in fraud_file)

all_intersections = set()

# Loop through each file from 1.txt to 3000.txt
for i in range(1, 3001):
    file_path = os.path.join(directory_path, f"{i}.txt")
    try:
        with open(file_path, 'r') as file:
            # Skip the first line and read the rest
            next(file)
            nodes = set(line.strip() for line in file)
            # Update the set with intersected nodes
            all_intersections.update(nodes.intersection(fraud_nodes))
    except FileNotFoundError:
        # If the file doesn't exist, move on to the next one
        continue

# Write all intersecting nodes to a single output file
output_file_path = os.path.join(directory_path, 'intercepted_nodes.txt')
with open(output_file_path, 'w') as output_file:
    for node in all_intersections:
        output_file.write(node + '\n')
