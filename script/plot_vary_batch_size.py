import os
import matplotlib.pyplot as plt

def extract_data(input_folder_path, application, group_list, precision_baseline, recall_baseline):
    precision_data = {}
    recall_data = {}

    for (time_slice_length, half_life_ratio, retirement_ratio), p_baseline, r_baseline in zip(group_list, precision_baseline, recall_baseline):
        folder_name = f"{time_slice_length}_{half_life_ratio}_{retirement_ratio}"
        folder_path = os.path.join(input_folder_path, folder_name, application)
        summary_file_path = os.path.join(folder_path, 'summary.txt')

        try:
            with open(summary_file_path, 'r') as file:
                lines = file.readlines()
                precision = float(lines[1].strip()) - p_baseline
                recall = float(lines[2].strip()) - r_baseline
                precision_data[folder_name] = precision
                recall_data[folder_name] = recall
        except FileNotFoundError:
            print(f"File not found: {summary_file_path}")
            precision_data[folder_name] = None
            recall_data[folder_name] = None

    return precision_data, recall_data

def plot_data(precision_data, recall_data, group_labels, dataset):
    plt.figure(figsize=(7, 5.2))
    plt.plot(group_labels, list(precision_data.values()), marker='o', label='Precision')
    plt.plot(group_labels, list(recall_data.values()), marker='x', label='Recall')
    plt.xlabel('Batch Size')
    plt.ylabel('Value')
    # plt.title(f'Precision and Recall for {dataset.capitalize()} Dataset')
    plt.xticks(ticks=range(len(group_labels)), labels=group_labels, rotation=45)
    plt.legend()
    plt.savefig(f"{dataset}_vary_batch_size.png")
    plt.close()

# Define parameters
datasets = [
    # 'bayc', 
    # 'terraforms', 
    # 'meebits', 
    'adr', 
    'ios'
    ]
application = 'rush'
group_list = [
    # (900, 1, 16),
    # (450, 2, 16),
    # (225, 4, 16),
    # (112, 8, 16),
    # (56, 16, 16),
    # (28, 32, 16),
    (60, 1, 16),
    (30, 2, 16),
    (12, 4, 16),
    (6, 8, 16),
    (3, 16, 16),
    # (1, 32, 16),
    # ... other groups ...
]

# group_list = [
    # (3600, 1, 16),
    # (1800, 2, 16),
    # (900, 4, 16),
    # (450, 8, 16),
    # (225, 16, 16),
    # (112, 32, 16),
# 
# ]

group_labels = [
    "1", 
    "2", 
    "4", 
    "8", 
    "16", 
    # "32", 
    ]  # Custom labels for each group

# group_labels = [
    # "56", 
    # "112", 
    # "225", 
    # "450", 
    # "900", 
    # "1800", 
    # "3600", 
    # "7200", 
    # "14400", 
    # "28800", 
    # ]  # Custom labels for each group
precision_baseline = [
    0.05, 
    0.04, 
    0.025, 
    0.025, 
    0.025, 
    0.025, 
    ]  # Example baseline precision values
recall_baseline = [
    0.06, 
    0.06, 
    0.05, 
    0.05, 
    0.04, 
    0.04, 
    ]  # Example baseline recall values
# recall_baseline = [
    # 0.00, 
    # 0.001, 
    # 0.01, 
    # 0.01, 
    # 0.02, 
    # 0.02, 
    # 0.03, 
    # 0.03, 
    # 0.00, 
    # 0.00, 
    # 0.00, 
    # ]  # Example baseline recall values


# Extract and plot data for each dataset
for dataset in datasets:
    input_folder_path = f'/data/yuhang/rush/{dataset}/'
    precision_data, recall_data = extract_data(input_folder_path, application, group_list, precision_baseline, recall_baseline)
    plot_data(precision_data, recall_data, group_labels, dataset)
    print(precision_data)
    print(recall_data)
