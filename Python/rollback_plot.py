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

def plot_data(ax, data, title, x_label, y_label, y_range):
    ax.set_title(title)
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_ylim(y_range[0], y_range[1])
    ax.plot(data[0], data[1])

def get_latest_filenames(num, log_type):
    date_format = "%Y.%m.%d-%H.%M.%S"
    date_strings = [filename[:find_nth(filename, "-", 2)] for filename in os.listdir(logs_path)]
    dates = [datetime.strptime(date_string, date_format) for date_string in date_strings]
    sorted_dates = sorted(dates, reverse=True)
    latest_dates = sorted_dates[:num]
    return [(logs_path + datetime.strftime(date, date_format) + "-" + log_type + "Log.txt") for date in latest_dates]

if __name__ == "__main__":
    short_options = "ln:t:"
    long_options = ["latest", "num=", "type="]
    full_cmd_arguments = sys.argv
    argument_list = full_cmd_arguments[1:]
    try:
        arguments, values = getopt.getopt(argument_list, short_options, long_options)
    except getopt.error as err:
        print(str(err))
        sys.exit(2)
    file_names = []
    if len(arguments) > 0:
        num_files = 1
        use_latest = False
        log_type = "Discrepancy"
        for current_argument, current_value in arguments:
            if current_argument in ("--latest", "-l"):
                use_latest = True
            if current_argument in ("--num", "-n"):
                num_files = int(current_value)
            if current_argument in ("--type", "-t"):
                log_type = current_value
        file_names = get_latest_filenames(num_files, log_type)
    else:
        file_names = sys.argv[1:]
        

    fig, ax = plt.subplots(4)
    for file_name in file_names:
        f = open(file_name)
        text_data = f.read()
        #print(text_data)
        time_data = text_data.split("Time: ")[1:]


        angle_data = parse_data_for_stat(time_data, "Angle: ", lambda x : math.degrees(x))
        if angle_data:
            plot_data(ax[0], angle_data, "Average rollback angle discrepancy", "Time (s)", "Angle (degrees)", (0, 50))

        distance_data = parse_data_for_stat(time_data, "Distance: ")
        if distance_data:
            plot_data(ax[1], distance_data, "Average rollback position discrepancy", "Time (s)", "Distance (cm)", (0, 100))

        precision_data = parse_data_for_stat(time_data, "Precision: ")
        if precision_data:
            plot_data(ax[2], precision_data, "Random Hit Precision", "Time (s)", "Precision", (0, 1))

        fudge_data = parse_data_for_stat(time_data, "OptimalFudge: ")
        if fudge_data:
            plot_data(ax[2], fudge_data, "Optimal Fudge Factor", "Time (s)", "Fudge Factor", (0, 1.0))

        transmission_data = parse_data_for_stat(time_data, "Transmission: ")
        if transmission_data:
            plot_data(ax[3], transmission_data, "Transmission Time", "Time (s)", "Time (s)", (0, 1.0))
    #print(time_data)
    plt.show()


