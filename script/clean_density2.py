# Assuming the input file is 'numbers.txt' and the output file will be 'numbers_with_average.txt'
import random

# Step 1: Calculate the average of all numbers
total_sum = 0
count = 0
with open('density_new.txt', 'r') as file:
    for line in file:
        try:
            # Convert each line to a float and add it to total_sum
            number = float(line.strip())
            total_sum += number
            count += 1
        except ValueError:
            # Skip lines that cannot be converted to a float
            continue

# Calculate the average, avoid division by zero
average = total_sum / count if count else 0

# Step 2: Add the average to each number and write to a new file
with open('density_new.txt', 'r') as file, open('density2.txt', 'w') as outfile:
    for line in file:
        try:
            number = float(line.strip())
            # Add the average to the original number
            new_number = number + random.uniform(0.4, 0.6)*average
            # Write the new number to the output file
            outfile.write(f"{new_number}\n")
        except ValueError:
            # Skip lines that cannot be converted to a float
            continue
