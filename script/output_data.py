import os

# Define your parameters
# times = [1, 60, 225, 450, 900, 1800, 3600, 7200, 14400]
# times = [900]
# times = [1800]
# times = [3600]
times = [8]
# times = [2]
r1_values = [4]
# r1_values = [1]
# r2_values = [16]
r2_values = [64]

datasets = [
    'dot',
    'terraforms',
    'bayc',
    'meebits',
    'artblock',
    # 'ios',
    # 'adr',
]

applications = [
    # '3_core'
    # 'cohen',
    # 'fraudar',
    # 'spade',
    'rush',
]

# Function to print or save data
def output_data(data, dataset, application):
    print(f"Data for Dataset: {dataset}, Application: {application}")
    for folder_name, metrics in data.items():
        print(f"Folder: {folder_name}, Avg Time: {metrics['avg_time']}, Precision: {metrics['precision']}, Recall: {metrics['recall']}")

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
                            f1 = 2*(precision * recall) /(precision + recall)
                            print('%.2f %.2f %.2f' % (precision, recall, f1), end = ' ')

                    except FileNotFoundError:
                        print(f"File not found: {summary_file_path}")

        # Output the data
        # output_data(data, dataset, application)
