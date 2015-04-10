all: libtiming

libtiming: timing.c Makefile
	$(CC) -fpic -O2 -shared -o ../lib/libtiming_papi.so timing.c -lrt -lpapi

test.link: libtiming
	$(CC) -std=gnu99 -O0 main.c -L../lib/ -ltiming_papi -lrt -lpapi -o test.link

test.exe: timing.c main.c Makefile
	$(CC) -std=gnu99 -O0 main.c timing.c -o $@ -lrt -lpapi
	
clean:
	rm -f l*.so test.exe
