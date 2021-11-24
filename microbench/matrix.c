#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <time.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define debug_print(...) fprintf(stderr, __VA_ARGS__)
#define N_CPU 4

int nfd = -1;
volatile unsigned long *task_core0 = NULL;
long task_cnt = 0;

void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *this_func, void *call_site)
{
    // inject to every function entry
    unsigned long val = *task_core0; // take pid on Lucene
    // printf("CPU 0's running task is pid:%d, tid:%d\n", (int)(val & (0xffffffff)), (int)(val >> 32));
    if ((int)(val & (0xffffffff)) != 0) // partner not idle, sleep
        ioctl(nfd, 0, NULL);            // get into nanonap
}

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
    while (1)
    {
        matrix();
    }
    return 0;   
}
