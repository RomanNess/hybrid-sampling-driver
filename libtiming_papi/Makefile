PREFIX=../..

all: libtiming.so libdummy.so

libtiming.so: timing.c
	$(CC) -fpic -O2 -shared -o libtiming.so timing.c -lrt -lpapi

libdummy.so: dummy.c
	$(CC) -fpic -O2 -shared -o $@ dummy.c -lrt


test.exe: main.cpp timing.c
#	$(CXX) -DMAXITERATIONS=100000000  main.cpp timing.c -o $@ -lrt -lpapi -O0
	$(CXX) main.cpp timing.c -o $@ -lrt -lpapi -O0

install: libdummy.so libtiming.so
	cp libtiming.so $(PREFIX)/lib/
	cp libdummy.so $(PREFIX)/lib/

clean:
	rm -f libtiming.so libdummy.so test.exe
