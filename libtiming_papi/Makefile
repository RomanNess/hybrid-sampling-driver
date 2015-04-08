all: libtiming.so

libtiming.so: timing.c Makefile
	$(CC) -fpic -O2 -shared -o libtiming.so timing.c -lrt -lpapi

test.exe: timing.c main.cpp Makefile
	$(CXX) main.cpp timing.c -o $@ -lrt -lpapi -O0

clean:
	rm -f libtiming.so test.exe
