import matplotlib.pyplot as plt
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
        float_vals = [float(v) for v in matches]
        if process_data_fn:
            float_vals = [process_data_fn(v) for v in float_vals]
        average = sum(float_vals) / len(float_vals)
        res[1].append(average)
    return res


def plot_data(ax, data, title, x_label, y_label, y_range):
    ax.set_title(title)
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_ylim(y_range[0], y_range[1])
    ax.plot(data[0], data[1])

def get_latest_filenames(num):
    date_format = "%Y.%m.%d-%H.%M.%S"
    date_strings = [filename[:-8] for filename in os.listdir(logs_path)]
    print(os.listdir(logs_path))
    dates = [datetime.strptime(date_string, date_format) for date_string in date_strings]
    sorted_dates = sorted(dates, reverse=True)
    latest_dates = sorted_dates[:num]
    return [(logs_path + datetime.strftime(date, date_format) + "-Log.txt") for date in latest_dates]

if __name__ == "__main__":
    short_options = "ln:"
    long_options = ["latest", "num="]
    full_cmd_arguments = sys.argv
    argument_list = full_cmd_arguments[1:]
    try:
        arguments, values = getopt.getopt(argument_list, short_options, long_options)
    except getopt.error as err:
        print(str(err))
        sys.exit(2)
    file_names = []
    print(arguments)
    if len(arguments) > 0:
        num_files = 1
        use_latest = False
        for current_argument, current_value in arguments:
            if current_argument in ("--latest", "-l"):
                use_latest = True
            if current_argument in ("--num", "-n"):
                num_files = int(current_value)
        file_names = get_latest_filenames(num_files)
    else:
        file_names = sys.argv[1:]
        

    fig, ax = plt.subplots(3)
    for file_name in file_names:
        f = open(file_name)
        text_data = f.read()
        #print(text_data)
        time_data = text_data.split("Time: ")[1:]


        angle_data = parse_data_for_stat(time_data, "Angle: ", lambda x : math.degrees(x))
        plot_data(ax[0], angle_data, "Average rollback angle discrepancy", "Time (s)", "Angle (degrees)", (0, 50))

        distance_data = parse_data_for_stat(time_data, "Distance: ")
        plot_data(ax[1], distance_data, "Average rollback position discrepancy", "Time (s)", "Distance (cm)", (0, 50))

        distance_data = parse_data_for_stat(time_data, "Precision: ")
        plot_data(ax[2], distance_data, "Random Hit Precision", "Time (s)", "Precision", (0, 1))
    #print(time_data)
    plt.show()


