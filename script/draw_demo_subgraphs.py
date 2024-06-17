import networkx as nx
import matplotlib.pyplot as plt

# Given data
data = """
3063206 3411352 1 954390459
3063206 3411352 1 954390597
3063206 3411352 1 954391002
3063206 4153860 1 954391444
3063206 4153860 1 954391568
3063206 3411352 1 954392415
3063206 3411352 1 954393619
3063206 5584629 1 954393966
3063206 4153860 1 954394402
3063206 3411352 1 954394882
3063206 5584629 1 954394999
3063206 3411352 1 954396039
3063206 4153860 1 954396164
3063206 4153860 1 954396366
3063206 3411352 1 954397199
3063206 3411352 1 954397322
3063206 4153860 1 954398105
3063206 4153860 1 954398330
3063206 5584629 1 954398452
3063206 5584629 1 954427773
3063206 5584629 1 954428390
"""

# Fraud node
fraud_node = '3063206'

# Create a MultiGraph
G = nx.MultiDiGraph()

# Parse the data and add edges to the graph
for line in data.strip().split('\n'):
    parts = line.split()
    G.add_edge(parts[0], parts[1])

# Draw the graph with curved edges
pos = nx.spring_layout(G)
plt.figure(figsize=(8, 6))

# Draw all nodes first
nx.draw_networkx_nodes(G, pos, node_color='blue', node_size=50)
# Highlight the fraud node in red
nx.draw_networkx_nodes(G, pos, nodelist=[fraud_node], node_color='red', node_size=100)

# Draw the edges with unique curves
for i, (u, v, k) in enumerate(G.edges(keys=True)):
    style = "arc3,rad=" + str( (i - (G.number_of_edges(u, v) - 1) / 2))
    nx.draw_networkx_edges(G, pos, edgelist=[(u, v)], connectionstyle=style, arrows=False)

plt.axis('off')  # No axis for a cleaner look
plt.savefig('graph.png', format='PNG')  # Save the graph to a file
plt.show()  # Show the graph in the output
