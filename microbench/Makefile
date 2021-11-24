all: matrix matrix_dynamic matrix_fixed matrix_refresh matrix_nonap

matrix: matrix.c
	gcc -o matrix -finstrument-functions matrix.c -lpthread

matrix_nonap: matrix_nonap.c
	gcc -o matrix_nonap matrix_nonap.c -lpthread

matrix_dynamic: matrix_dynamic.c
	gcc -o matrix_dynamic -finstrument-functions matrix_dynamic.c -lpthread

matrix_fixed: matrix_fixed.c
	gcc -o matrix_fixed -finstrument-functions matrix_fixed.c -lpthread

matrix_refresh: matrix_refresh.c
	gcc -o matrix_refresh -finstrument-functions matrix_refresh.c -lpthread

clean-user:
	rm -f matrix matrix_dynamic matrix_fixed matrix_refresh

clean: clean-user 
