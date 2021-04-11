import matplotlib.pyplot as plt
import shutil
import numpy as np
import os
import sys, getopt
import re
import math
from datetime import datetime

"""
"""

logs_path = "../FPSTemplate/DebugLogs/"

def parse_data_for_stat(time_data, stat_key, process_data_fn = None):
    res = ([], [])
    for d in time_data:
        time_str = d[:d.find('\n')]
        res[0].append(float(time_str))
        p = re.compile('(?<=' + stat_key + ').*(?=\n)')
        matches = p.findall(d)
        if not matches:
            return None
        float_vals = [float(v) for v in matches]
        if process_data_fn:
            float_vals = [process_data_fn(v) for v in float_vals]
        average = sum(float_vals) / len(float_vals)
        res[1].append(average)
    return res

def find_nth(string, substring, n):
   if (n == 1):
       return string.find(substring)
   else:
       return string.find(substring, find_nth(string, substring, n - 1) + 1)

def plot_data(ax, data, title, x_label, y_label, y_range, color):
    ax.set_title(title)
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_ylim(y_range[0], y_range[1])
    ax.plot(data[0], data[1], color=color)

def get_latest_filenames(num, log_type):
    date_format = "%Y.%m.%d-%H.%M.%S"
    files = [f for f in os.listdir(logs_path) if not os.path.isdir(f) and f[-1] == 't']
    date_strings = [filename[:find_nth(filename, "-", 2)] for filename in files]
    dates = [datetime.strptime(date_string, date_format) for date_string in date_strings]
    sorted_dates = sorted(dates, reverse=True)
    latest_dates = sorted_dates[:num]
    return [(logs_path + datetime.strftime(date, date_format) + "-" + log_type + "Log.txt") for date in latest_dates]

def make_single_fig(file_names, data_types, legend = []):
    fig, ax = plt.subplots(len(data_types))
    if not isinstance(ax, list):
        ax = [ax]
    colors = ((0.1, 0.6, 0.4, 1.0), (0.7, 0.7, 0.0, 1.0), (0.7, 0.45, 0.0, 1.0), (0.8, 0.1, 0.15, 1.0))
    for i, file_name in enumerate(file_names):
        used_axes = 0
        f = open(file_name)
        text_data = f.read()
        #print(text_data)
        time_data = text_data.split("Time: ")[1:]


        angle_data = parse_data_for_stat(time_data, "Angle: ", lambda x : math.degrees(x))
        if angle_data and "Angle" in data_types:
            plot_data(ax[used_axes], angle_data, "Average rollback angle discrepancy", "Time (s)", "Angle (degrees)", (0, 50), colors[i])
            used_axes += 1

        distance_data = parse_data_for_stat(time_data, "Distance: ")
        if distance_data and "Distance" in data_types:
            plot_data(ax[used_axes], distance_data, "Average rollback position discrepancy", "Time (s)", "Distance (cm)", (0, 300), colors[i])
            used_axes += 1

        precision_data = parse_data_for_stat(time_data, "Precision: ")
        if precision_data and "Precision" in data_types:
            plot_data(ax[used_axes], precision_data, "Random Hit Precision", "Time (s)", "Precision", (0, 1), colors[i])
            used_axes += 1

        fudge_data = parse_data_for_stat(time_data, "OptimalFudge: ")
        if fudge_data and "Fudge" in data_types:
            plot_data(ax[used_axes], fudge_data, "Optimal Fudge Factor", "Time (s)", "Fudge Factor", (0, 1.0), colors[i])
            used_axes += 1

        transmission_data = parse_data_for_stat(time_data, "Transmission: ")
        if transmission_data and "Transmission" in data_types:
            plot_data(ax[used_axes], transmission_data, "Transmission Time", "Time (s)", "Time (s)", (0, 1.0), colors[i])
            used_axes += 1

    for ind_ax in ax:
        ind_ax.legend(legend, loc=1)

    return fig

def get_full_file_sets(full_directory):
    files = [f for f in os.listdir(full_directory) if not os.path.isdir(f) and f[-1] == 't']
    files = sorted(files)

    file_sets = []
    prev_config = None
    for f in files:
        separated = f.split("-")
        config_str = "".join(separated[:-2])
        if config_str != prev_config:
            prev_config = config_str
            file_sets.append([])
        file_sets[-1].append(f)
    
    return file_sets

def get_file_set_latencies(file_set):
    return [f.split('-')[-2] + "ms" for f in file_set]


if __name__ == "__main__":
    short_options = "ln:t:fd:"
    long_options = ["latest", "num=", "type=", "full", "directory="]
    full_cmd_arguments = sys.argv
    argument_list = full_cmd_arguments[1:]
    try:
        arguments, values = getopt.getopt(argument_list, short_options, long_options)
    except getopt.error as err:
        print(str(err))
        sys.exit(2)

    is_full = False
    file_names = []
    file_sets = []
    if len(arguments) > 0:
        num_files = 1
        use_latest = False
        log_type = "Discrepancy"
        full_directory = None
        for current_argument, current_value in arguments:
            if current_argument in ("--latest", "-l"):
                use_latest = True
            if current_argument in ("--full", "-f"):
                is_full = True
            if current_argument in ("--directory", "-d"):
                full_directory = current_value
            if current_argument in ("--num", "-n"):
                num_files = int(current_value)
            if current_argument in ("--type", "-t"):
                log_type = current_value
        if not is_full:
            file_names = get_latest_filenames(num_files, log_type)
        else:
            file_sets = get_full_file_sets(full_directory)
    else:
        file_names = sys.argv[1:]
        
    if not is_full:
        fig = make_single_fig(file_names, ["Angle", "Distance", "Precision"])
        plt.show()
    else:
        images_path = full_directory + "\img"
        if os.path.exists(images_path):
            shutil.rmtree(images_path)
        os.mkdir(images_path)
        data_types = ["Angle", "Distance", "Precision"]
        for data_type in data_types:
            for file_set in file_sets:
                config = "".join(file_set[0].split('-')[:-2])
                latencies = get_file_set_latencies(file_set)
                fig = make_single_fig([full_directory + "\\" + f for f in file_set], [data_type], latencies)
                fig.savefig(images_path + '\\' + config + data_type + ".png")
