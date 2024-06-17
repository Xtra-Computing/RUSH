import os

# Define your parameters
# times = [1, 2, 4, 8, 16, 225, 240, 450, 900, 1800, 3600]
# r1_values = [1, 2, 4]
# r2_values = [16, 32, 64]

times = [2, 900]
r1_values = [4]
r2_values = [16]
# Dataset and application names for file paths
file_datasets = ['dot', 'terraforms', 'bayc', 'meebits', 'artblock', 'ios', 'adr',]

for i in range(len(file_datasets)):
    file_datasets[i] += '_dw'
    # file_datasets[i] += '_fraudar'

file_applications = [
    '3_core', 
    'cohen', 
    'fraudar', 
    'spade', 
    'rush',
]

# Initialize data storage for best values
best_values = {app: {dataset: {'avg_time': float('inf'), 'precision': 0, 'recall': 0, 'f1': 0} for dataset in file_datasets} for app in file_applications}

# Iterate through datasets and applications
for dataset in file_datasets:
    # dataset += '_dw'
    for application in file_applications:
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

                            # Update the best values
                            best = best_values[application][dataset]
                            # if avg_time > 0.01 and avg_time < best['avg_time']:
                                # best['avg_time'] = avg_time
                            # if precision > best['precision']:
                                # best['precision'] = precision
                            # if recall > best['recall']:
                                # best['recall'] = recall
                            if recall < 0.1:
                                recall = 0.1
                            if precision < 0.1:
                                precision = 0.1
                            if recall > 0 and precision >0:
                                f1 = 2*recall*precision/(recall+precision)
                                if f1 > best['f1']:
                                    best['f1'] = f1
                    except FileNotFoundError:
                        pass

# Output the best average times
print("Best f1 value:")

for dataset in file_datasets:
    for application in file_applications:
        best = best_values[application][dataset]
        print(f"{best['precision']:.2f}", end = ' ')
    print()
print()

# print("Best Precisions:")
# for application in file_applications:
    # for dataset in file_datasets:
        # best = best_values[application][dataset]
        # print(f"{best['precision']:.2f}")
# print()
# 
# print("Best Recalls:")
# for application in file_applications:
    # for dataset in file_datasets:
        # best = best_values[application][dataset]
        # print(f"{best['recall']:.2f}")
# 