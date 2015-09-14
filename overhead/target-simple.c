#ifdef META_BENCHMARK
	#define TARGET_ITERATIONS 1000*1000*100
#else
	#define TARGET_ITERATIONS 1000*10
#endif

void rec1() {
}

int main() {
	for (int i = 0; i < TARGET_ITERATIONS; i++) {
		rec1();
	}
	return 0;
}
