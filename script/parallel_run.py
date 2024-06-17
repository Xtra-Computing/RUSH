import subprocess
import itertools
from multiprocessing import Pool
import os

cmd_list = [
    'burst_mining',
    'core_mining',
    'cohensiveness'
    'spade'
]

application_list = [
    'rush',
    '3_core',
    'cohen',
    'spade'
]

def run_process(args):
    cmd = ["/home/yuhang/Work/BurstingGraphMining/build/burst_mining"] + list(map(str, args))
    subprocess.run(cmd)

def construct_output_directory(base_dir, input_path, time_slice, half_life, retirement, application):
    output_dir = f"{base_dir}{input_path}/{time_slice}_{half_life}_{retirement}/{application}"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    return output_dir

def main():
    input_file_data = [
        # ('bayc', 1650364600),
        # ('terraforms', 1650620294),
        # ('ios', 954388451),
        # ('adr', 954112538),
        # ('meebits', 1651992148),
        # ('bayc_fraudar', 1650364600),
        # ('terraforms_fraudar', 1650620294),
        # ('meebits_fraudar', 1651992148),
        ('ios_fraudar', 954388451),
        ('adr_fraudar', 954112538),
    ]  

    # time_slice_length_list =[225*x for x in [1,2,4]]   # Time slice lengths in seconds
    time_slice_length_list =[2]   # Time slice lengths in seconds
    # half_life_ratio_list = [1, 2, 4]  # Example half-life ratios
    half_life_ratio_list = [4]  # Example half-life ratios
    # retirement_ratio_list = [16, 32, 64]  # Example retirement ratios
    retirement_ratio_list = [16]  # Example retirement ratios
    data_directory = "/data/yuhang/rush/"  # Base output directory

    args_list = []
    for (input_path, start_time), time_slice_length, half_life_ratio, retirement_ratio in itertools.product(input_file_data, time_slice_length_list, half_life_ratio_list, retirement_ratio_list):
        input_file1 = f"{data_directory}{input_path}/static.txt"
        input_file2 = f"{data_directory}{input_path}/increment.txt"
        application = 'rush'
        output_dir = construct_output_directory(data_directory, input_path, time_slice_length, half_life_ratio, retirement_ratio, application)
        args = [input_file1, input_file2, start_time, time_slice_length, half_life_ratio, retirement_ratio, output_dir]
        args_list.append(args)

    with Pool() as pool:
        pool.map(run_process, args_list)

if __name__ == "__main__":
    main()
