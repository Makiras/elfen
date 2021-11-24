import matplotlib.pyplot as plt


def draw_utilization_ipc(x, y, xlabel, ylabel, title):
    plt.plot(x, y, "-o")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.show()


def draw_99lt_batchcount(x, y, xlabel, ylabel, title):
    plt.plot(x, y, "-o")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.show()


def draw_latency_distribution(x, y, xlabel, ylabel, title):
    plt.plot(x, y, "-o")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.show()
