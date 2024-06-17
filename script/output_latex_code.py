import os

# Define your parameters
times = [1, 60, 225, 450, 900, 1800, 3600, 7200, 14400]
r1_values = [1, 2, 4]
r2_values = [16, 32, 64]

# Original dataset and application names for file paths
file_datasets = ['bayc', 'terraforms', 'meebits', 'adr', 'ios']
file_applications = ['3_core', 'cohen', 'spade', 'rush']

# New dataset and application names for output
output_datasets = {'bayc': 'BAYC', 'terraforms': 'Terraforms', 'meebits': 'Meebits', 'adr': 'Grab-Android', 'ios': 'Grab-IOS'}
output_applications = {'rush': 'RUSH', '3_core': 'BC', 'cohen': 'BCH', 'spade': 'Spade'}

# Initialize data storage for best values
best_values = {app: {dataset: {'avg_time': float('inf'), 'precision': 0, 'recall': 0} for dataset in file_datasets} for app in file_applications}

# Iterate through datasets and applications
for dataset in file_datasets:
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
                            if avg_time < best['avg_time']:
                                best['avg_time'] = avg_time
                            if precision > best['precision']:
                                best['precision'] = precision
                            if recall > best['recall']:
                                best['recall'] = recall
                    except FileNotFoundError:
                        pass

# Function to generate LaTeX table
def generate_latex_table(metric, data):
    header = " & ".join(["\\textit{" + output_applications[app] + "}" for app in file_applications])
    table = "\\begin{table*}[t]\n\\centering\n\\begin{minipage}{0.32\\textwidth}\n\\centering\n"
    table += "\\caption{" + metric.capitalize() + ".}\n\\resizebox{\\textwidth}{!}{\n"
    table += "\\begin{tabular}{c|" + 'c' * len(file_applications) + "}\n\\hline\n\\textit{} & " + header + " \\\\ \\hline\n"
    
    for dataset in file_datasets:
        row_data = []
        for app in file_applications:
            value = data[app][dataset][metric]
            # Determine if this is the best value
            is_best = value == min(data[a][dataset][metric] for a in file_applications) if metric == 'avg_time' else value == max(data[a][dataset][metric] for a in file_applications)
            formatted_value = f"\\textbf{{{value:.2f}}}" if is_best else f"{value:.2f}"
            row_data.append(formatted_value if value != float('inf') and value > 0 else '')
        row = " & ".join(row_data)
        table += "\\textit{" + output_datasets[dataset] + "} & " + row + " \\\\ \\hline\n"
    
    table += "\\end{tabular}\n}\n\\end{minipage}\n\\hfill\n"
    return table

# Generate LaTeX tables
avg_time_table = generate_latex_table('avg_time', best_values)
precision_table = generate_latex_table('precision', best_values)
recall_table = generate_latex_table('recall', best_values)

latex_output = avg_time_table + precision_table + recall_table + "\\end{table*}"

# Print or save the LaTeX tables
print(latex_output)
