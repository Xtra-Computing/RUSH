import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from datetime import datetime, timedelta

# Step 1: Read the density values
density_values = np.loadtxt('density2.txt')

# Calculate the average of all nodes in the plot
average_density = np.mean(density_values)

# Step 2: Generate the time series for the x-axis
start_time = datetime(2000, 3, 15)
time_series = [start_time + timedelta(seconds=20*i) for i in range(len(density_values))]

# Step 3: Read 'increment.txt' and 'fraud.txt'
increment_nodes = [line.split()[:2] for line in open('/data/yuhang/rush/ios/increment.txt', 'r')]
fraud_nodes = set(line.strip() for line in open('/data/yuhang/rush/ios/fraud.txt', 'r'))

# Find lines where conditions are met
fraud_points = []
for i, (node1, node2) in enumerate(increment_nodes):
    if i >= len(density_values):
        break  # Skip if i is greater than the length of density values
    if (node1 in fraud_nodes or node2 in fraud_nodes) and density_values[i] > 3 * average_density:
        fraud_points.append((time_series[i], 1.2 * density_values[i]))

# Step 4: Plot the line chart
plt.figure(figsize=(10, 6))
plt.plot(time_series, density_values, label='Density')

# Step 5 & 6: Highlight with red dots
if fraud_points:
    fraud_times, fraud_values = zip(*fraud_points)  # Unpack the times and values
    plt.scatter(fraud_times, fraud_values, color='red', zorder=5)

plt.xlabel('Time')
plt.ylabel('Density')
plt.title('Density of Graphs Over Time')
plt.xticks(rotation=45)
plt.tight_layout()
plt.legend()
plt.show()
plt.savefig("density.png")
