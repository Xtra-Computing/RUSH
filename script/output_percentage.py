import os

def calculate_burst_percentage(increment_file_path, directory_path, fraud_file_path, threshold):
    # Initialize variables
    accumulative_weight = {}
    burst_nodes = set()
    total_nodes = set()
    fraud_nodes = set()

    # Read and store fraud nodes
    with open(fraud_file_path, 'r') as file:
        for line in file:
            node = line.strip()
            fraud_nodes.add(node)

    # Process increment file
    with open(increment_file_path, 'r') as file:
        for i, line in enumerate(file):
            # print(i)
            parts = line.strip().split(' ')
            # print(parts)
            if len(parts) != 4:
                continue  # skip lines that do not have exactly 4 elements

            source, target, weight, _ = parts
            weight = float(weight)
            total_nodes.update([source, target])

            # Update accumulative weight
            for node in [source, target]:
                accumulative_weight[node] = accumulative_weight.get(node, 0) + weight

                # Read i.txt and check for source/target node
                ith_file_path = os.path.join(directory_path, f'{i}.txt')
                with open(ith_file_path, 'r') as ith_file:
                    # print(ith_file.read())
                    if node in ith_file.read() and accumulative_weight[node] > threshold:
                        burst_nodes.add(node)

    # Calculate the percentage of burst nodes in all nodes
    burst_percentage = len(burst_nodes) / len(total_nodes) if total_nodes else 0

    # Calculate the percentage of fraud nodes in burst nodes
    burst_fraud_intersection = burst_nodes.intersection(fraud_nodes)
    fraud_percentage = len(burst_fraud_intersection) / len(burst_nodes) if burst_nodes else 0
    fraud_percentage2 = len(burst_fraud_intersection) / len(fraud_nodes) if burst_nodes else 0

    return burst_percentage, fraud_percentage, fraud_percentage2

# Define the file paths and threshold
# dataset = 'bayc'
# dataset = 'terraforms'
# dataset = 'artblock'
# dataset = 'dot'
# dataset = 'meebits'
# dataset = 'ios'
dataset = 'adr'
increment_file = f'/data/yuhang/rush/{dataset}/increment.txt'  # Update with the actual path to the increment.txt file
# nodes_directory = f'/data/yuhang/rush/{dataset}/900_4_16/rush/'  # Update with the actual directory path containing node files
nodes_directory = f'/data/yuhang/rush/{dataset}/8_4_16/rush/'  # Update with the actual directory path containing node files
fraud_file = f'/data/yuhang/rush/{dataset}/fraud.txt'  # Update with the actual path to the fraud.txt file
threshold = 1  # Define the threshold for burst detection

# Calculate the burst percentage and fraud percentage
burst_percentage, fraud_percentage, f2 = calculate_burst_percentage(increment_file, nodes_directory, fraud_file, threshold)

print(f"Burst Node Percentage: {burst_percentage * 100:.2f}%")
print(f"Fraud Node Percentage among Burst Nodes: {fraud_percentage * 100:.2f}%")
print(f2)
