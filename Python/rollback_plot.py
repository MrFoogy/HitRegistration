import matplotlib.pyplot as plt
import numpy as np
import sys
import re
import math

"""
"""

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


if __name__ == "__main__":
    for file_name in sys.argv[1:]:
        f = open(file_name)
        text_data = f.read()
        #print(text_data)
        time_data = text_data.split("Time: ")[1:]

        fig, ax = plt.subplots(2)

        angle_data = parse_data_for_stat(time_data, "Angle: ", lambda x : math.degrees(x))
        plot_data(ax[0], angle_data, "Average rollback angle discrepancy", "Time (s)", "Angle (degrees)", (0, 50))

        distance_data = parse_data_for_stat(time_data, "Distance: ")
        plot_data(ax[1], distance_data, "Average rollback position discrepancy", "Time (s)", "Distance (cm)", (0, 50))
    #print(time_data)
    plt.show()


