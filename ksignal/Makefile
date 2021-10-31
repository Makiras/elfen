KDIR = /lib/modules/`uname -r`/build
obj-m := ksignal.o
M := make -C ${KDIR} M=`pwd`

all:
	${M} modules

clean-kernel:
	${M} clean

user: test_ksignal
test_ksignal: test_ksignal.c
	gcc -o test_ksignal test_ksignal.c -lpthread
clean-user:
	rm test_ksignal

clean: clean-user clean-kernel
