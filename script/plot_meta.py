import os
import matplotlib.pyplot as plt

# Define your parameters
times = [3600, 7200, 14400]
r1_values = [1, 2, 4]
r2_values = [16, 32, 64]

datasets = [
    'bayc',
    'terraforms',
    'meebits',
    # 'adr',
    # 'ios'
]
applications = [
    'rush',
    '3_core'
]

# Iterate through datasets and applications
for dataset in datasets:
    for application in applications:
        data = {}  # Initialize data storage for each dataset and application
        input_folder_path = f'/data/yuhang/rush/{dataset}/'  # Update with your folder path

        # Iterate through the combinations of time, r1, and r2
        for time in times:
            for r1 in r1_values:
                for r2 in r2_values:
                    folder_name = f"{time}_{r1}_{r2}"
                    folder_path = os.path.join(input_folder_path, folder_name, application)
                    summary_file_path = os.path.join(folder_path, 'summary.txt')

                    # Read the summary.txt file
                    try:
                        with open(summary_file_path, 'r') as file:
                            lines = file.readlines()
                            avg_time = float(lines[0].strip())
                            precision = float(lines[1].strip())
                            recall = float(lines[2].strip())

                            # Store the data
                            data[folder_name] = {'avg_time': avg_time, 'precision': precision, 'recall': recall}
                    except FileNotFoundError:
                        print(f"File not found: {summary_file_path}")

        # Create a directory for the plots
        plot_dir = f'./{dataset}/{application}'
        os.makedirs(plot_dir, exist_ok=True)

        # Plotting
        for metric in ['avg_time', 'precision', 'recall']:
            plt.figure()
            folder_names = list(data.keys())
            values = [data[folder_name][metric] for folder_name in folder_names]
            plt.bar(folder_names, values)
            plt.xticks(rotation=45, ha='right')  # Rotate x-axis labels for better readability
            plt.title(f"{metric} by folder for {dataset}-{application}")
            plt.xlabel('Folder Path')
            plt.ylabel(metric)
            plt.tight_layout()  # Adjust layout for better fit

            # Save the plot in the designated directory
            plt.savefig(os.path.join(plot_dir, f'{metric}_by_folder.png'))

# This script now saves the plots in different folders based on the dataset and application.
