import re

# Regular expression to match lines with a numeric prefix followed by ".txt:"
pattern = re.compile(r'^(\d+)\.txt:\s*(\d+)')

# Read the file and filter valid lines
valid_lines = []
with open('densities.txt', 'r') as file:
    for line in file:
        match = pattern.match(line)
        if match:
            # Extract the numeric prefix and the number, convert them to integers for sorting
            prefix, number = int(match.group(1)), int(match.group(2))
            valid_lines.append((prefix, number))

# Sort the valid lines by the numeric prefix
sorted_lines = sorted(valid_lines, key=lambda x: x[0])

# Write the sorted numbers to an output file
with open('density_new.txt', 'w') as file:
    for _, number in sorted_lines:
        file.write(f"{number}\n")
