#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

#define debug_print(...) fprintf(stderr, __VA_ARGS__)
#define N_CPU 4

// nanonap
int nfd = -1;
volatile unsigned long *task_core0 = NULL;
long task_cnt = 0;

int __attribute__((no_instrument_function))
fetch_signal_phy_address(char *path, int nr_cpu, unsigned long *signal_addr)
{
    int fd, nr_read;
    char buf[1024];
    char *cur = buf;

    if ((fd = open(path, O_RDONLY)) < 0)
    {
        fprintf(stderr, "Can't open signal address file %s\n", path);
        return 1;
    }
    nr_read = read(fd, buf, 1024);
    debug_print("read %d bytes %s from shim_sginal\n", nr_read, buf);
    signal_addr[0] = atol(buf);
    for (int i = 1; i < nr_cpu; i = i + 1)
    {
        while (*(cur++) != ',')
            ;
        signal_addr[i] = atol(cur);
    }
    close(fd);
    return 0;
}

int __attribute__((no_instrument_function))
map_signal_phy_address(int cpuid, unsigned long *signal_addr, unsigned long **signal)
{

    unsigned long phy_addr = signal_addr[cpuid];
    unsigned long mmap_offset = phy_addr & ~(0x1000 - 1);
    int mmap_size = 0x1000;
    int signal_offset = phy_addr & (0x1000 - 1);
    int mmap_fd;

    if ((mmap_fd = open("/dev/mem", O_RDONLY)) < 0)
    {
        fprintf(stderr, "Can't open /dev/mem");
        return 1;
    }
    char *mmap_addr = mmap(0, mmap_size, PROT_READ, MAP_SHARED, mmap_fd, mmap_offset);
    if (mmap_addr == MAP_FAILED)
    {
        fprintf(stderr, "Can't mmap /dev/mem");
        return 1;
    }
    *signal = (unsigned long *)(mmap_addr + signal_offset);
    debug_print("map cpu%d signal_addr:0x%lx to virtual_addr:%p\n", cpuid, phy_addr, *signal);
    return 0;
}

// lucene state
unsigned long *lucene_signal_base;
volatile unsigned int *lucene_queue_signal;
volatile unsigned long *lucene_signal_buf;
unsigned long cr_start_timestamp;
unsigned long last_request;
// budget, ms * 1M * cpu_freq (constant_tsc need)
const unsigned long budget = 8 * 1000 * 1000 * 2.9; // 8ms, 2.9Ghz

enum Estatus
{
    NORMAL,
    NEW_PERIOD,
    CO_RUNNING,
    OTHERS,
};

enum Estatus lane_status = NORMAL;

static __attribute__((no_instrument_function)) unsigned long __inline__ rdtsc(void)
{
    unsigned int tickl, tickh;
    __asm__ __volatile__("rdtscp"
                         : "=a"(tickl), "=d"(tickh)::"%ecx");
    return ((uint64_t)tickh << 32) | tickl;
}

void __attribute__((no_instrument_function))
init_lucene_signal()
{
    int fd = open("../lucene/util/lucene_signal", O_RDWR);
    if (fd == -1)
    {
        err(1, "Can't open ../lucene/util/lucene_signal\n");
        exit(1);
    }
    lucene_signal_base = (unsigned long *)mmap(0, 0x1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (lucene_signal_base == MAP_FAILED)
    {
        err(1, "Can't mmap ../lucene/util/lucene_signal\n");
        exit(1);
    }
    lucene_signal_buf = lucene_signal_base + 1;
    lucene_queue_signal = (unsigned int *)lucene_signal_base;
    for (int i = 0; i < 17; i++)
    {
        lucene_signal_base[i] = 0;
    }
}

// perf get ipc

int perf_fd = -1;
int lc_read_id = 0;
const float ref_ipc = 1.8;

struct read_format
{
    uint64_t nr;
    uint64_t values[2];
};

struct perf_event_attr attr;
struct read_format lc_read[2];

int __attribute__((no_instrument_function))
perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

void __attribute__((no_instrument_function))
init_perf_var()
{
    int LC_pid = 0, LC_cpu = 0;
    puts("Input LC pid, LC cpu:");
    // scanf("%d %d", &LC_pid, &LC_cpu);
    LC_pid = 9619;  // lucene

    memset(&attr, 0, sizeof(struct perf_event_attr));
    attr.size = sizeof(struct perf_event_attr);
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_INSTRUCTIONS;
    attr.disabled = 1;
    attr.read_format = PERF_FORMAT_GROUP;
    perf_fd = perf_event_open(&attr, LC_pid, LC_cpu, -1, 0);
    if (perf_fd < 0)
    {
        perror("perf_event_open 1");
        exit(1);
    }

    memset(&attr, 0, sizeof(struct perf_event_attr));
    attr.size = sizeof(struct perf_event_attr);
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_CPU_CYCLES;
    attr.disabled = 1;
    if (perf_event_open(&attr, LC_pid, LC_cpu, perf_fd, 0) < 0)
    {
        perror("perf_event_open 2");
        exit(1);
    }

    ioctl(perf_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    read(perf_fd, &lc_read[0], sizeof(struct read_format));
}

double __attribute__((no_instrument_function))
cacl_ipc()
{
    lc_read_id = (lc_read_id + 1) % 2;
    read(perf_fd, &lc_read[lc_read_id], sizeof(struct read_format));
    // printf("%ld, %ld, %ld\n", lc_read[lc_read_id].nr, lc_read[lc_read_id].values[0], lc_read[lc_read_id].values[1]);
    double ipc = (double)(lc_read[lc_read_id].values[0] - lc_read[(lc_read_id + 1) % 2].values[0]) / (double)(lc_read[lc_read_id].values[1] - lc_read[(lc_read_id + 1) % 2].values[1]);
    // printf("ipc:%f\n", ipc);
    // ipc will be -nan
    return isnan(ipc) ? 0 : ipc;
}

// inject

void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *this_func, void *call_site)
{
    // inject to every function entry
    unsigned long val = *task_core0; // take pid on Lucene
    unsigned int queue_len = *lucene_queue_signal;
    double calc_ipc = 0;
    // printf("CPU 0's running task is pid:%d, tid:%d\n", (int)(val & (0xffffffff)), (int)(val >> 32));

    unsigned int pid = (unsigned int)(val & (0xffffffff));
    if (pid == 0 && queue_len == 0) // partner is idle, queue len = 0
        lane_status = NEW_PERIOD;
    else if (pid != 0) // partner is running
    {
        switch (lane_status)
        {
        case NORMAL:
            ioctl(nfd, 0, NULL); // get into nanonap
            break;
        case NEW_PERIOD:
            lane_status = CO_RUNNING;
            last_request = *lucene_signal_buf; // task id
            cr_start_timestamp = rdtsc();
            break;
        case CO_RUNNING:
            if (*lucene_signal_buf != last_request && queue_len == 0) // new task
            {
                last_request = *lucene_signal_buf;
                cr_start_timestamp = rdtsc(); // refresh timestamp
                return;
            }
            // if timestamp is refreshed, it won't reach budget
            // calc real budget time with latency critical app ipc
            // what if (ref-cacl_ipc) < 0 ? it will be positive feedback
            calc_ipc = ref_ipc - cacl_ipc();
            if (rdtsc() - cr_start_timestamp > budget * (ref_ipc / calc_ipc) && calc_ipc > 0)
                lane_status = NORMAL;
            break;
        }
    }
}

// main

int a[5][5] = {
    {1, 2, 3, 4, 5},
    {6, 7, 8, 9, 10},
    {11, 12, 13, 14, 15},
    {16, 17, 18, 19, 20},
    {21, 22, 23, 24, 25}};
int b[5][5] = {
    {1, 2, 3, 4, 5},
    {6, 7, 8, 9, 10},
    {11, 12, 13, 14, 15},
    {16, 17, 18, 19, 20},
    {21, 22, 23, 24, 25}};
int c[5][5] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}};

void __attribute__((no_instrument_function)) show_taskcnt(int x)
{
    printf("calc matrix %ld times.\n", task_cnt);
    exit(0);
}

void matrix()
{
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            for (int k = 0; k < 5; k++)
            {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    task_cnt++;
    return;
}

int __attribute__((no_instrument_function))
main(int argc, char **argv)
{
    signal(SIGINT, show_taskcnt);
    unsigned long signal_phy_addr[N_CPU];
    fetch_signal_phy_address("/sys/module/ksignal/parameters/task_signal", N_CPU, signal_phy_addr);
    for (int i = 0; i < N_CPU; i++)
        printf("CPU %d task signal at phy addr 0x%lx\n", i, signal_phy_addr[i]);
    //let's monitor CPU 0
    map_signal_phy_address(0, signal_phy_addr, (unsigned long **)(&task_core0));
    printf("Map CPU 0's task signal to address %p\n", task_core0);
    unsigned long val = *task_core0;
    printf("CPU 0's running task is pid:%d, tid:%d\n", (int)(val & (0xffffffff)), (int)(val >> 32));
    // let's get nanonap
    nfd = open("/dev/nanonap", O_RDONLY | O_CLOEXEC);
    if (nfd < 0)
    {
        fprintf(stderr, "Can't open /dev/nanonap\n");
        exit(0);
    }
    // let's get lucene signal
    init_lucene_signal();
    // let's get perf var
    init_perf_var();

    while (1)
    {
        matrix();
    }
}
