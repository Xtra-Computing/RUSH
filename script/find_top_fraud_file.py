# Define the path to the fraud file and the directory containing the other files
fraud_file_path = '/data/yuhang/rush/ios/fraud.txt'
files_directory = '/data/yuhang/rush/ios/2_4_16/rush'

# Step 1: Read fraud nodes into a set
with open(fraud_file_path, 'r') as file:
    fraud_nodes = set(file.read().splitlines())

# Initialize a dictionary to hold the count of fraud nodes for each file
fraud_count_per_file = {}

# Step 2: Loop through each file
for i in range(1, 3001):
    file_path = f'{files_directory}/{i}.txt'
    try:
        with open(file_path, 'r') as file:
            # Step 3: Read nodes in the current file
            nodes = file.read().splitlines()
            # Step 4: Count the fraud nodes in the current file
            fraud_count = sum(node in fraud_nodes for node in nodes)
            # Store the count
            fraud_count_per_file[i] = fraud_count
    except FileNotFoundError:
        print(f'File {i}.txt not found.')
        continue

top_10_files_with_most_fraud_nodes = sorted(fraud_count_per_file.items(), key=lambda item: item[1], reverse=True)[:10]

# Print the top 10 files and their fraud node counts
print("Top 10 files with the most fraud nodes:")
for file_number, count in top_10_files_with_most_fraud_nodes:
    print(f'File {file_number}.txt: {count} fraud nodes')