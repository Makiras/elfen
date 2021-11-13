#include <sched.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h> /* For SYS_xxx definitions */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sched.h>
#include <err.h>
#include <sys/time.h>
#include <sys/resource.h>

unsigned long *lucene_signal_base;
unsigned int *lucene_queue_signal;
unsigned long *lucene_signal_buf;

void init_lucene_signal()
{
    int fd = open("./lucene_signal", O_RDWR);
    if (fd == -1)
    {
        err(1, "Can't open ./lucene_signal\n");
        exit(1);
    }
    lucene_signal_base = (unsigned long *)mmap(0, 0x1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (lucene_signal_base == MAP_FAILED)
    {
        err(1, "Can't mmap ./lucene_signal\n");
        exit(1);
    }
    lucene_signal_buf = lucene_signal_base + 1;
    lucene_queue_signal = (unsigned int *)lucene_signal_base;
    for (int i = 0; i < 17; i++)
    {
        lucene_signal_base[i] = 0;
    }
}

int main()
{
    init_lucene_signal();
    while (1)
    {
        printf("\r%d\t\t%lld\t\t", *lucene_queue_signal, *lucene_signal_buf);
    }
    return 0;
}