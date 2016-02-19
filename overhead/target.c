#ifdef META_BENCHMARK
	#define TARGET_ITERATIONS 1000*1000*10
#else
	#define TARGET_ITERATIONS 1000
#endif

void rec10() {
	asm ("");	// cannot optimize it away now, eh gcc?
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

int main() {
	for (int i = 0; i < TARGET_ITERATIONS; i++) {
		rec1();
	}
	return 0;
}
