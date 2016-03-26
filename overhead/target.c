#ifdef META_BENCHMARK
	#define TARGET_ITERATIONS 1000*1000*100
#else
	#define TARGET_ITERATIONS 1000
#endif

#include <stdio.h>

void rec10() {
	asm ("");	// avoid optimization
}

void rec9() {
	rec10();
}

void rec8() {
	rec9();
}

void rec7() {
	rec8();
}

void rec6() {
	rec7();
}

void rec5() {
	rec6();
}

void rec4() {
	rec5();
}

void rec3() {
	rec4();
}

void rec2() {
	rec3();
}

void rec1() {
	rec2();
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
