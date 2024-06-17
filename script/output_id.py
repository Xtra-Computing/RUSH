import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Function to read the increment file and return a DataFrame with density data and transaction data
def read_increment_file(filepath):
    transactions = pd.read_csv(filepath, sep=' ', names=['source', 'target', 'weight', 'timestamp'])
    return transactions

# Function to read the fraud file and return a set of fraud nodes
def read_fraud_file(filepath):
    fraud_nodes = pd.read_csv(filepath, names=['node']).squeeze()
    return set(fraud_nodes)

# Function to merge and average densities every 10 timestamps
def merge_and_average_densities(transactions):
    transactions['timestamp_group'] = transactions['timestamp'] // 10
    grouped_density = transactions.groupby('timestamp_group').size().reset_index(name='density')
    grouped_density['density'] = grouped_density['density'] / 10  # Averaging the density
    return grouped_density

# Function to identify burst timestamps
def identify_burst_timestamps(grouped_density):
    average_density = grouped_density['density'].mean()
    burst_timestamps = grouped_density[grouped_density['density'] > 2 * average_density]['timestamp_group']
    return set(burst_timestamps)

# Function to identify burst fraud nodes
def identify_burst_fraud_nodes(transactions, fraud_nodes, burst_timestamps):
    # Identifying timestamps with fraud nodes
    fraud_transactions = transactions[transactions['source'].isin(fraud_nodes) | transactions['target'].isin(fraud_nodes)]
    fraud_transactions['timestamp_group'] = fraud_transactions['timestamp'] // 10
    burst_fraud_timestamps = fraud_transactions[fraud_transactions['timestamp_group'].isin(burst_timestamps)]['timestamp_group']
    return set(burst_fraud_timestamps)

# Function to plot the merged density and burst fraud nodes
def plot_density(grouped_density, burst_fraud_timestamps, output_file):
    plt.figure(figsize=(10, 5))
    
    # Plot the merged density
    plt.plot(grouped_density['timestamp_group'], grouped_density['density'], label='Merged Density')

    # Prepare data for burst fraud nodes
    max_density = grouped_density['density'].max()
    fraud_timestamps = []
    fraud_densities = []
    for timestamp in burst_fraud_timestamps:
        if timestamp in grouped_density['timestamp_group'].values:
            fraud_timestamps.append(timestamp)
            # Finding the corresponding density and multiplying by 1.2
            fraud_density = grouped_density[grouped_density['timestamp_group'] == timestamp]['density'].iloc[0] * 1.2
            fraud_densities.append(fraud_density)

    # Plot burst fraud nodes
    plt.plot(fraud_timestamps, fraud_densities, 'ro', label='Burst Fraud Nodes')

    plt.xlabel('Merged Timestamps')
    plt.ylabel('Average Density')
    plt.legend()
    plt.savefig(output_file)

# Function to identify burst fraud nodes and return their IDs
def identify_burst_fraud_nodes_and_ids(transactions, fraud_nodes, burst_timestamps):
    fraud_transactions = transactions[transactions['source'].isin(fraud_nodes) | transactions['target'].isin(fraud_nodes)]
    fraud_transactions['timestamp_group'] = fraud_transactions['timestamp'] // 10
    burst_fraud_transactions = fraud_transactions[fraud_transactions['timestamp_group'].isin(burst_timestamps)]
    
    # Getting unique burst fraud node IDs
    burst_fraud_node_ids = set(burst_fraud_transactions['source']).union(set(burst_fraud_transactions['target']))
    burst_fraud_node_ids = burst_fraud_node_ids.intersection(fraud_nodes)

    return burst_fraud_node_ids, set(burst_fraud_transactions['timestamp_group'])

# Function to print burst fraud node IDs
def print_burst_fraud_node_ids(burst_fraud_node_ids):
    print("Burst Fraud Node IDs:")
    for node_id in burst_fraud_node_ids:
        print(node_id)

# Modify the main function accordingly
def main():
    transactions = read_increment_file('/data/yuhang/rush/ios/test.txt')
    fraud_nodes = read_fraud_file('/data/yuhang/rush/ios/fraud.txt')
    grouped_density = merge_and_average_densities(transactions)
    burst_timestamps = identify_burst_timestamps(grouped_density)
    burst_fraud_node_ids, burst_fraud_timestamps = identify_burst_fraud_nodes_and_ids(transactions, fraud_nodes, burst_timestamps)

    # Print the IDs of burst fraud nodes
    print_burst_fraud_node_ids(burst_fraud_node_ids)

    plot_density(grouped_density, burst_fraud_timestamps, 'output.png')

if __name__ == "__main__":
    main()
