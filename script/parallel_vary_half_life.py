import subprocess
import os
from multiprocessing import Pool
import itertools
import re

def run_process(args):
    output_dir = args[-1]
    summary_file_path = os.path.join(output_dir, 'summary.txt')

    # Check if summary.txt already exists
    # if os.path.exists(summary_file_path):
        # print(f"Skipping as summary file already exists in {output_dir}")
        # return

    cmd = ["/home/yuhang/Work/BurstingGraphMining/build/burst_mining"] + list(map(str, args))
    subprocess.run(cmd)
    # After running the command, call process_files to generate summary
    directory = os.path.dirname(args[0])

    # Construct the new file path
    new_file_path = os.path.join(directory, "fraud.txt")
    process_files(output_dir, new_file_path, args[1])

def construct_output_directory(base_dir, input_path, time_slice, half_life, retirement, application):
    output_dir = f"{base_dir}{input_path}/{time_slice}_{half_life}_{retirement}/{application}"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    return output_dir

def process_files(folder_path, fraud_file_path, increment_file_path):
    # Initialize variables
    total_time = 0
    file_count = 0
    catch_nodes = set()
    fraud_nodes = set()
    correct_count = 0
    fraud_node_count = 0

    # Read and store fraud nodes
    with open(fraud_file_path, 'r') as file:
        for line in file:
            fraud_node = line.strip()
            if fraud_node:
                fraud_nodes.add(fraud_node)

    # Process increment file
    with open(increment_file_path, 'r') as increment_file:
        count = 0
        for line in increment_file:
            # print(count)
            nodes = line.split()[:2]
            weight = line.split()[2]
            is_fraud_line = any(node in fraud_nodes for node in nodes)
            file_name = f"{count}.txt"
            count += 1

            file_path = os.path.join(folder_path, file_name)
            try:
                with open(file_path, 'r') as file:
                    # Read the first line and extract time
                    first_line = file.readline()
                    time_match = re.search(r'Elapsed time: ([\d.]+) ms', first_line)
                    if time_match:
                        total_time += float(time_match.group(1))
                        file_count += 1

                    # Process node IDs
                    current_file_nodes = set()
                    for line in file:
                        node_id = line.strip()
                        catch_nodes.add(node_id)
                        current_file_nodes.add(node_id)
            except FileNotFoundError:
                break

            is_fraud_file = any(node in current_file_nodes for node in nodes)
            # if is_fraud_file:
            # print(is_fraud_file, end = ' ')
            # print(is_fraud_line)
            if is_fraud_line == is_fraud_file:
                correct_count += 1
            elif float(weight) < 1:
                correct_count += 1
            elif float(weight) > 10 and is_fraud_file:
                correct_count += 1
            

    # Calculate average time
    average_time = total_time / file_count if file_count else 0
    fraud_node_count = len(catch_nodes.intersection(fraud_nodes))

    summary_file_path = folder_path + '/summary.txt'

    with open(summary_file_path, 'w') as f:
        f.write(str(average_time) + '\n')
        f.write(str(correct_count/file_count) + '\n')
        f.write(str(fraud_node_count/len(fraud_nodes)) + '\n')

def main():
    input_file_data = [
        ('bayc', 1650364600),
        ('terraforms', 1650620294),
        ('meebits', 1651992148),
        # ('artblock', 1651907349),
        # ('dot', 1642688831),
        # ('ios', 954388451),
        # ('adr', 954112538),
        # Add more pairs as needed
    ]  

    group_list = [
        (12, 4, 16),
        (25, 4, 16),
        (50, 4, 16),
        (110, 4, 16),
        (225, 4, 16),
        (450, 4, 16),
        (900, 4, 16),
        (1800, 4, 16),
        (3600, 4, 16),
        (7200, 4, 16),
        # (1, 4, 16),
        # (2, 4, 16),
        # (4, 4, 16),
        # (8, 4, 16),
        # (15, 4, 16),
        # (30, 4, 16),
        # (60, 4, 16),
        # (120, 4, 16),
       # ... other groups ...
    ]
    data_directory = "/data/yuhang/rush/"
    application = 'rush'

    args_list = []
    for (input_path, start_time), (time_slice_length, half_life_ratio, retirement_ratio) in itertools.product(input_file_data, group_list):
        input_file1 = f"{data_directory}{input_path}/static.txt"
        input_file2 = f"{data_directory}{input_path}/increment.txt"
        output_dir = construct_output_directory(data_directory, input_path, time_slice_length, half_life_ratio, retirement_ratio, application)
        args = [input_file1, input_file2, start_time, time_slice_length, half_life_ratio, retirement_ratio, output_dir]
        args_list.append(args)

    with Pool() as pool:
        pool.map(run_process, args_list)

if __name__ == "__main__":
    main()
