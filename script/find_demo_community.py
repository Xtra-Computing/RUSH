import networkx as nx
from collections import Counter

# Step 1: Read the fraud nodes
def read_fraud_nodes(fraud_file):
    with open(fraud_file, 'r') as file:
        return set(file.read().splitlines())

# Step 2: Identify the top five fraud nodes in 'increment.txt'
def find_top_fraud_nodes(fraud_nodes, increment_file):
    fraud_node_counts = Counter()
    with open(increment_file, 'r') as file:
        for line in file:
            source, target, _, _ = line.split()
            if source in fraud_nodes:
                fraud_node_counts[source] += 1
            if target in fraud_nodes:
                fraud_node_counts[target] += 1
    return [node for node, count in fraud_node_counts.most_common(20)]

# Step 3: For each top fraud node, find all one or two-hop nodes and relevant edges
def find_edges_and_output(fraud_nodes, increment_file, top_fraud_nodes):
    G = nx.Graph()
    
    # Read all edges to construct the graph
    with open(increment_file, 'r') as file:
        for line in file:
            source, target, weight, _ = line.split()
            G.add_edge(source, target, weight=float(weight))
    
    for fraud_node in top_fraud_nodes:
        # Find one-hop and two-hop nodes
        one_hop = set(G.neighbors(fraud_node))
        two_hop = set()
        for node in one_hop:
            two_hop.update(G.neighbors(node))
        nodes1 = one_hop.union(two_hop, set([fraud_node]))
        
        # Find and output all relevant edges
        with open(f"demo/{fraud_node}_edges.txt", 'w') as outfile:
            with open(increment_file, 'r') as file:
                for line in file:
                    source, target, weight, timestamp = line.split()
                    if source in nodes1 and target in nodes1:
                        outfile.write(line)

fraud_file = '/data/yuhang/rush/ios/fraud.txt'
increment_file = '/data/yuhang/rush/ios/increment.txt'

fraud_nodes = read_fraud_nodes(fraud_file)
top_fraud_nodes = find_top_fraud_nodes(fraud_nodes, increment_file)
find_edges_and_output(fraud_nodes, increment_file, top_fraud_nodes)
