import os
import re

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
            except FileNotFoundError:
                break

            is_fraud_file = any(node in current_file_nodes for node in nodes)
            # if is_fraud_file:
            # print(is_fraud_file, end = ' ')
            # print(is_fraud_line)
            if is_fraud_line == is_fraud_file:
                correct_count += 1

    # Calculate average time
    average_time = total_time / file_count if file_count else 0
    fraud_node_count = len(catch_nodes.intersection(fraud_nodes))

    summary_file_path = folder_path + '/summary.txt'

    with open(summary_file_path, 'w') as f:
        f.write(str(average_time) + '\n')
        f.write(str(correct_count/file_count) + '\n')
        f.write(str(fraud_node_count/len(fraud_nodes)) + '\n')


# Execute the function in your environment with the actual file paths
# average_time, correct_count, fraud_node_count = process_files(folder_path, fraud_file_path, increment_file_path)
# print(f"Average Time: {average_time} ms, Correct Count: {correct_count}, Fraud Node Count: {fraud_node_count}")

def main():
    input_folder_path = '/data/yuhang/rush/ios/'  # Update with your folder path
    fraud_file_path = '/data/yuhang/rush/ios/fraud.txt'  # Update with your fraud nodes file path

    for time in [60*x for x in [1, 2, 4]]:
        for r1 in [1, 2, 4]:
            for r2 in [16, 32, 64]:
                folder_path = input_folder_path + str(time) + '_' + str(r1) + '_' + str(r2) + '/spade'
                print(folder_path)
                increment_file_path = os.path.join(input_folder_path, 'increment.txt')
                process_files(folder_path, fraud_file_path, increment_file_path)


if __name__ == "__main__":
    main()
