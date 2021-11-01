


matrix: matrix.c
	gcc -o matrix -finstrument-functions matrix.c -lpthread
	gcc -o matrix_nonap matrix.c -lpthread

clean-user:
	rm test_nanonap

clean: clean-user clean-kernel
