#!/bin/python3

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
from lib import *

mpl.use("TkAgg")


def draw_process_time(file_name: str, save_name: str):
    """
    Draw the COCO result.
    """
    (l_res, r_res) = parse_conclusion_csv(file_name)

    fig, x_t_process = plt.subplots()
    x_utilization = x_t_process.twinx()

    # draw process time (50,95,99)
    axis_process_time(x_t_process, r_res)

    # draw cpu utilization
    axis_LC_cpu_utilization(x_utilization, r_res)
    axis_total_cpu_utilization(x_utilization, r_res)

    plt.title("process time (50,95,99)")
    plt.grid(True)
    # plt.show()
    plt.savefig(save_name)
    plt.close()


def draw_ipc(file_name: str, save_name: str):
    (l_res, r_res) = parse_conclusion_csv(file_name)

    fig, x_ipc = plt.subplots()
    x_utilization = x_ipc.twinx()

    # draw process time (50,95,99)
    axis_LC_ipc(x_ipc, r_res)
    axis_BW_ipc(x_ipc, r_res)

    # draw cpu utilization
    axis_LC_cpu_utilization(x_utilization, r_res)
    axis_total_cpu_utilization(x_utilization, r_res)

    plt.title("IPC")
    plt.grid(True)
    # plt.show()
    plt.savefig(save_name)
    plt.close()


def draw_cpi(file_name: str, save_name: str):
    (l_res, r_res) = parse_conclusion_csv(file_name)

    fig, x_cpi = plt.subplots()
    x_utilization = x_cpi.twinx()

    axis_BW_cpi(x_cpi, r_res)

    # draw cpu utilization
    axis_LC_cpu_utilization(x_utilization, r_res)
    axis_total_cpu_utilization(x_utilization, r_res)

    plt.title("Matrix CPS")
    plt.grid(True)
    # plt.show()
    plt.savefig(save_name)
    plt.close()


if __name__ == "__main__":
    todo_list = [
        "nonap-co",
        "coco",
        "idle-co",
        "fixed-10",
        "refresh-10",
        "dynamic-10-18",
        "dynamic-10-20",
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
        draw_cpi(
            "../../data/{}-data/conclusion.csv".format(i),
            "../picture/cpi/{}-cpi.png".format(i),
        )
