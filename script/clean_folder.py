import os
import shutil

def clean_directory(directory):
    # Check if directory exists
    if not os.path.exists(directory):
        print(f"The directory {directory} does not exist.")
        return

    # Recursively clean the directory
    _recursive_clean(directory)

    # Check if the original directory is now empty
    if os.listdir(directory) == []:
        shutil.rmtree(directory)
        print(f"Deleted folder: {directory}")

def _recursive_clean(folder):
    # List all items in the folder
    items = os.listdir(folder)

    # If the folder only contains 'summary.txt', delete the folder
    if len(items) == 1 and items[0] == 'summary.txt':
        shutil.rmtree(folder)
        print(f"Deleted folder: {folder}")
        # Check if the parent folder is now empty, and clean it as well
        parent_folder = os.path.dirname(folder)
        if parent_folder and os.listdir(parent_folder) == []:
            _recursive_clean(parent_folder)
    else:
        # If the folder has more items, recursively check each item (if they are folders)
        for item in items:
            item_path = os.path.join(folder, item)
            if os.path.isdir(item_path):
                _recursive_clean(item_path)

    if os.listdir(folder) == []:
        shutil.rmtree(folder)
        print(f"Deleted folder: {folder}")

# Usage
directory_path = '/data/yuhang/rush'  # Replace with your directory path
clean_directory(directory_path)
