import os

def process_files(folder_path):
    total_elapsed_time = 0.0
    file_count = 0
    all_nodes = []

    while True:
        file_path = os.path.join(folder_path, f"{file_count + 1}.txt")
        if not os.path.exists(file_path):
            break  # Stop if the file does not exist

        with open(file_path, 'r') as file:
            lines = file.readlines()
            elapsed_time_line = lines[1]
            elapsed_time = float(elapsed_time_line.split()[2])
            total_elapsed_time += elapsed_time

            for line in lines[2:]:  # Skip the first two lines
                source_node, target_node, _ = line.split()
                all_nodes.append(source_node + ' ' + target_node)

        file_count += 1

    if file_count == 0:
        return 0, []  # No files processed

    average_elapsed_time = total_elapsed_time / file_count
    return average_elapsed_time, all_nodes

def write_summary(folder_path, average_elapsed_time, all_nodes):
    summary_file_path = os.path.join(folder_path, 'summary.txt')
    with open(summary_file_path, 'w') as summary_file:
        summary_file.write(f"Average elapsed time: {average_elapsed_time} ms\n")
        for node_pair in all_nodes:
            summary_file.write(node_pair + '\n')

def main():
    input_folder_path = '/data/yuhang/Rush_experiment/bayc/'  # Update with your folder path

    for time in [3600, 7200, 14400]:
        for r1 in [1, 2, 4]:
            for r2 in [16, 32, 64]:
                folder_path = input_folder_path + str(time) + '_' + str(r1) + '_' + str(r2)
                average_elapsed_time, all_nodes = process_files(folder_path)
                write_summary(folder_path, average_elapsed_time, all_nodes)

if __name__ == "__main__":
    main()