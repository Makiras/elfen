import matplotlib.pyplot as plt
import numpy as np
import csv


def parse_conclusion_csv(file_name: str):
    """
    format:
    QPS, RealQPS, p50, p95, p99, l50, l95, l99, utiliztion, IPC,
    send time,recv time,
    optional:
    matrix, user_t, sys_t, b_utilization
    """
    l_reader = csv.DictReader(open(file_name, "r"))
    r_reader: dict = {}
    if l_reader.fieldnames is None:
        exit(1)
    else:
        print(l_reader.fieldnames)
        for label in l_reader.fieldnames:
            r_reader[label] = []
    for l in l_reader:
        for k in l.keys():
            r_reader[k].append(float(l[k] if l[k] != None else 0))
    return l_reader, r_reader


def axis_process_time(x_t_process, r_res):
    x_t_process.plot(r_res["QPS"], r_res["l50"], "-x")
    x_t_process.plot(r_res["QPS"], r_res["l95"], "-x")
    x_t_process.plot(r_res["QPS"], r_res["l99"], "-x")
    x_t_process.set_xlabel("QPS")
    x_t_process.set_ylabel("process time (ms)")
    x_t_process.legend(["50%", "95%", "99%"], loc="upper left")
    x_t_process.set_ylim(0, 200)


def axis_LC_cpu_utilization(x_utilization, r_res):
    # draw LC cpu utilization
    x_utilization.plot(r_res["QPS"], r_res["utilization"], "-")
    x_utilization.fill_between(r_res["QPS"], r_res["utilization"], 0, alpha=0.1)
    x_utilization.set_ylabel("cpu utilization")
    x_utilization.set_ylim(0, 2.7)

    # draw hline for cpu trubo max
    x_utilization.axhline(y=3.9 / 2.9, color="r", linestyle="--")


def axis_total_cpu_utilization(x_utilization, r_res):
    # x_utilization.plot(r_res["QPS"], r_res["b_utilization"], "-x")
    total_utilization = np.array(r_res["utilization"]) + np.array(
        r_res["b_utilization"]
    ) * np.array(r_res["user_t"]) / (
        np.array(r_res["user_t"]) + np.array(r_res["sys_t"])
    )
    x_utilization.plot(r_res["QPS"], total_utilization, "-")
    x_utilization.fill_between(
        r_res["QPS"], total_utilization, r_res["utilization"], alpha=0.1
    )
    x_utilization.legend(["LC", "1core max", "total"], loc="upper right")


def axis_LC_ipc(x_ipc, r_res):
    x_ipc.plot(r_res["QPS"], r_res["IPC"], "-x")
    x_ipc.set_xlabel("QPS")
    x_ipc.set_ylabel("IPC")
    x_ipc.legend(["IPC"], loc="upper left")
    x_ipc.set_ylim(0, 4)


def axis_BW_ipc(x_ipc, r_res):
    max_speed = 2452851  # calc matrix per second
    max_ipc = 3.62  # only matrix run has ipc 3.62

    equivalent_ipc = (
        (np.array(r_res["matrix"]) / np.array(r_res["user_t"])) / max_speed * max_ipc
    )

    x_ipc.plot(r_res["QPS"], equivalent_ipc, "-x")
    x_ipc.set_xlabel("QPS")
    x_ipc.set_ylabel("IPC")
    x_ipc.legend(["IPC", "Matrix eq_ipc"], loc="upper left")
    x_ipc.set_ylim(0, 4)


def axis_BW_cpi(x_ipc, r_res):
    max_speed = 2452851  # calc matrix per second
    max_ipc = 3.62  # only matrix run has ipc 3.62

    user_ratio = (np.array(r_res["user_t"]) * max_speed) / np.array(r_res["matrix"])
    total_ratio = (np.array(r_res["recv time"]) * max_speed) / np.array(r_res["matrix"])

    x_ipc.plot(r_res["QPS"], user_ratio, "-x")
    x_ipc.plot(r_res["QPS"], total_ratio, "-x")
    x_ipc.set_xlabel("QPS")
    x_ipc.set_ylabel("CPS Ratio")
    x_ipc.legend(["user_ratio", "total_ratio"], loc="upper left")
    x_ipc.set_ylim(0, 10)
