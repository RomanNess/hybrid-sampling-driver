#ifdef META_BENCHMARK
	#define TARGET_ITERATIONS 1000*1000*1000
#else
	#define TARGET_ITERATIONS 1000*10
#endif

#include <stdio.h>

void rec1() {
}

int main(int argc, char** argv) {

    int numIterations;
    if (argc < 2 || sscanf (argv[1], "%i", &numIterations)!=1) {
        numIterations=TARGET_ITERATIONS;
    }

	for (int i = 0; i < numIterations; i++) {
		rec1();
	}
	return 0;
}
