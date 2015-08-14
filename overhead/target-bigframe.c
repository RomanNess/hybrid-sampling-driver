#define ALLOC_SIZE 100000

void rec10() {
	int tmp10[ALLOC_SIZE];
}

void rec9() {
	int tmp9[ALLOC_SIZE];
	rec10();
}

void rec8() {
	int tmp8[ALLOC_SIZE];
	rec9();
}

void rec7() {
	int tmp7[ALLOC_SIZE];
	rec8();
}

void rec6() {
	int tmp6[ALLOC_SIZE];
	rec7();
}

void rec5() {
	int tmp5[ALLOC_SIZE];
	rec6();
}

void rec4() {
	int tmp4[ALLOC_SIZE];
	rec5();
}

void rec3() {
	int tmp3[ALLOC_SIZE];
	rec4();
}

void rec2() {
	int tmp2[ALLOC_SIZE];
	rec3();
}

void rec1() {
	int tmp1[ALLOC_SIZE];
	rec2();
}

int main() {

	for (int i = 0; i < 1000; i++) {
		rec1();
	}
	return 0;
}
