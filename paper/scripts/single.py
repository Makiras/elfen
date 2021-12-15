#!/bin/python3

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
from lib import *

mpl.use("TkAgg")


def draw_process_time(file_name: str, save_name: str):
    (l_res, r_res) = parse_conclusion_csv(file_name)

    fig, x_t_process = plt.subplots()
    x_utilization = x_t_process.twinx()

    # draw process time (50,95,99)
    axis_process_time(x_t_process, r_res)

    # draw cpu utilization
    axis_LC_cpu_utilization(x_utilization, r_res)

    plt.title("process time (50,95,99)")
    plt.grid(True)
    # plt.show()
    plt.savefig(save_name)

def draw_ipc(file_name: str, save_name: str):
    (l_res, r_res) = parse_conclusion_csv(file_name)

    fig, x_ipc = plt.subplots()
    x_utilization = x_ipc.twinx()

    # draw process time (50,95,99)
    axis_LC_ipc(x_ipc, r_res)

    # draw cpu utilization
    axis_LC_cpu_utilization(x_utilization, r_res)

    plt.title("IPC")
    plt.grid(True)
    # plt.show()
    plt.savefig(save_name)


if __name__ == "__main__":
    todo_list = [
        "single-co",
        "single-nap",
    ]
    for i in todo_list:
        draw_process_time(
            "../../data/{}-data/conclusion.csv".format(i),
            "../picture/process_time/{}-process-time.png".format(i),
        )
        draw_ipc(
            "../../data/{}-data/conclusion.csv".format(i),
            "../picture/ipc/{}-ipc.png".format(i),
        )
