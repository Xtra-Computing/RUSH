def process_file_to_uniform_weight(input_file_path, output_file_path):
    with open(input_file_path, 'r') as file, open(output_file_path, 'w') as outfile:
        for line in file:
            parts = line.strip().split()
            if len(parts) != 4:
                continue  # Skip lines that do not have 4 elements

            source, target, _, timestamp = parts
            # Set weight to 1
            outfile.write(f"{source} {target} 1 {timestamp}\n")

# Example usage
input_file_path = 'original.txt'  # Replace with your input file path
output_file_path = 'increment.txt'  # Replace with your desired output file path
process_file_to_uniform_weight(input_file_path, output_file_path)
