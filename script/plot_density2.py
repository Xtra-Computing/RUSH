import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from datetime import datetime, timedelta
from matplotlib.dates import DateFormatter, HourLocator

# Ensure that the font properties are set globally for all plots
plt.rcParams.update({'font.size': 22, 'font.family': 'Times New Roman'})

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
    if (node1 in fraud_nodes or node2 in fraud_nodes) and density_values[i] > 2 * average_density:
        fraud_points.append((time_series[i], 1 * density_values[i]))  # Adjust value for fraud points

# Step 4: Plot the line chart
plt.figure(figsize=(15, 5))
# plt.plot(time_series, density_values, label='Density', linewidth=2)
plt.plot(time_series, density_values, linewidth=2)
# plt.axis('off')
plt.gca().set_xticks([])

# Highlight fraud nodes with red dots and add to legend
if fraud_points:
    fraud_times, fraud_values = zip(*fraud_points)  # Unpack the times and values
    plt.scatter(fraud_times, fraud_values, color='red', zorder=5) #, label='Fraud Cases')

# Formatting the x-axis to show a time duration of 4 hours between each axis
# plt.gca().xaxis.set_major_locator(HourLocator(interval=4))
# plt.gca().xaxis.set_major_formatter(DateFormatter('%H:%M'))
# plt.gca().xaxis.set([])
# plt.set_xtics([])

plt.scatter([], [], s=100, color='red', label='Type 1')  # s is the size of the point
plt.scatter([], [], s=100, color='blue', label='Type 2')
plt.scatter([], [], s=100, color='green', label='Type 3')

# Place legend in the top left of the plot with specific font size and 3 columns
plt.legend(loc='upper left', fontsize=22, ncol=3)

# Adding legend to the top left of the figure

# Naming the axes and setting their labels
plt.xlabel('Mocked Time')
plt.ylabel('Density')

# Setting the title and adjusting the layout
# plt.title('Density of Graphs Over Time', fontsize=30)
# plt.xticks(rotation=45)
plt.tight_layout()

# Adding the legend with the specified font size
plt.legend(fontsize=22)

# Display before saving to ensure the figure is saved correctly
plt.show()

# Save the figure
plt.savefig("density_over_time.pdf")
