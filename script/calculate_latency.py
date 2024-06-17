import os

def calculate_average_latency(increment_file_path, directory_path, fraud_file_path, default_latency):
    # Initialize dictionaries to store occurrence and caught timestamps
    occurrence_timestamps = {}
    caught_timestamps = {}
    latencies = []
    fraud_nodes = []
    
    # Read and store fraud nodes with their occurrence timestamps
    with open(fraud_file_path, 'r') as file:
        for line in file:
            fraud_node = line.strip()
            fraud_nodes.append(fraud_node)

    # Process increment file to record occurrence timestamps and update caught timestamps
    with open(increment_file_path, 'r') as file:
        for i, line in enumerate(file):
            # print(i)
            parts = line.strip().split(' ')
            if len(parts) != 4:
                continue  # skip lines that do not have exactly 4 elements

            source, target, weight, timestamp = parts
            timestamp = int(timestamp)
            weight = float(weight)
            
            if weight > 1:
                if source in fraud_nodes:
                    occurrence_timestamps[source] = timestamp
                if target in fraud_nodes:
                    occurrence_timestamps[target] = timestamp
            
            # Read ith file and check for fraud nodes to update caught timestamp
            ith_file_path = os.path.join(directory_path, f'{i}.txt')
            if os.path.exists(ith_file_path):
                with open(ith_file_path, 'r') as ith_file:
                    # print(ith_file_path)
                    ith_nodes = set(ith_file.read().splitlines())
                    intersected_fraud_nodes = ith_nodes.intersection(fraud_nodes)
                    for node in intersected_fraud_nodes:
                        if node not in caught_timestamps:  # Update only if not caught before
                            if node in occurrence_timestamps:
                                caught_timestamps[node] = timestamp
                                print(timestamp, occurrence_timestamps[node])
                                latencies.append(timestamp - occurrence_timestamps[node])

    # Calculate latencies for fraud nodes that were caught
    # for node, occ_time in occurrence_timestamps.items():
        # if node in caught_timestamps:
            # latencies.append(caught_timestamps[node] - occ_time)
        # else:
            # Apply default latency for nodes that were not caught
            # latencies.append(default_latency)

    # Calculate the average latency
    print(latencies)
    average_latency = sum(latencies) / len(latencies) if latencies else 0
    return average_latency

# Define file paths and default latency value
dataset = 'bayc'
increment_file_path = f'/data/yuhang/rush/{dataset}/increment.txt'  # Update with the actual path to the increment.txt file
# directory_path = f'/data/yuhang/rush/{dataset}/900_4_16/rush/'  # Update with the actual directory path containing node files
directory_path = f'/data/yuhang/rush/{dataset}/3600_1_16/3_core/'  # Update with the actual directory path containing node files
fraud_file_path = f'/data/yuhang/rush/{dataset}/fraud.txt'  # Update with the actual path to the fraud.txt file
default_latency_value = 10000  # Replace with an appropriate default value for your use case

# Calculate the average caught time difference for all shown fraud nodes
average_latency = calculate_average_latency(increment_file_path, directory_path, fraud_file_path, default_latency_value)

print(f"Average Latency for Detected Fraud Nodes: {average_latency}ms")
