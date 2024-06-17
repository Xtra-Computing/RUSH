import os
import csv

def read_fraud_nodes(fraud_file):
    with open(fraud_file, 'r') as file:
        return set(line.strip() for line in file)

def check_fraud_in_file(file_path, node1, node2):
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for line in lines[2:]:  # Skip elapsed time line
            file_node1, file_node2, _ = line.split()
            if file_node1 == node1 and file_node2 == node2:
                return True
    return False

def process_files(folder_path, fraud_nodes, increment_file):
    files_processed = 0
    correct_files = 0
    total_elapsed_time = 0.0
    found_fraud_nodes = [] 

    with open(increment_file, 'r') as inc_file:
        for line in inc_file:
            node1, node2, _, _ = line.strip().split()
            file_path = os.path.join(folder_path, f"{files_processed + 1}.txt")

            contains_fraud = False
            # Read elapsed time and check for fraud
            if os.path.exists(file_path):
                with open(file_path, 'r') as file:
                    lines = file.readlines()
                    elapsed_line = lines[1]
                    elapsed_time = float(elapsed_line.split()[2])
                    total_elapsed_time += elapsed_time

                    for line in lines[2:]:
                        file_node1, file_node2, _ = line.split()
                        if file_node1 == node1 or file_node1 == node2:
                            contains_fraud == True
                            print(True)
                        if file_node2 == node1 or file_node2 == node2:
                            contains_fraud == True
                            print(True)
                        found_fraud_nodes.append(file_node1)
                        found_fraud_nodes.append(file_node2)

            is_fraud = node1 in fraud_nodes or node2 in fraud_nodes
            if contains_fraud == is_fraud:
                correct_files += 1
            files_processed += 1
            print(files_processed)
            # print(contains_fraud, end=' ')
            # print(is_fraud)

    precision = correct_files / files_processed if files_processed > 0 else 0
    found_fraud_nodes_set = set(found_fraud_nodes)
    recall = len(found_fraud_nodes_set & fraud_nodes) / len(fraud_nodes) if fraud_nodes else 0

    return files_processed, correct_files, total_elapsed_time, precision, recall

def write_summary(folder_path, summary_data):
    summary_file = os.path.join(folder_path, 'summary.csv')
    with open(summary_file, 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Total Files', 'Total Elapsed Time (ms)', 'Average Elapsed Time (ms)', 'Precision', 'Recall'])
        writer.writerow(summary_data)

def main():
    input_folder_path = '/data/yuhang/Rush_experiment/bayc/'  # Update with your folder path
    fraud_file = '/data/yuhang/Rush_experiment/bayc/fraud.txt'  # Update with your fraud nodes file path

    for time in [3600*24, 7200*24, 14400*24]:
        for r1 in [1, 2, 4]:
            for r2 in [16, 32, 64]:
                folder_path = input_folder_path + str(time) + '_' + str(r1) + '_' + str(r2)
                print(folder_path)
                increment_file = os.path.join(input_folder_path, 'increment.txt')
                fraud_nodes = read_fraud_nodes(fraud_file)
                files_processed, correct_files, total_elapsed_time, precision, recall = process_files(folder_path, fraud_nodes, increment_file)

                average_elapsed_time = total_elapsed_time / files_processed if files_processed > 0 else 0

                summary_data = [files_processed, total_elapsed_time, average_elapsed_time, precision, recall]
                print(f"{time}_{r1}_{r2}: Precision = {precision}, Recall = {recall}, avg_time = {average_elapsed_time}")
                write_summary(folder_path, summary_data)
