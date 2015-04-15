void rec10() {}

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

	for (int i = 0; i < 1000; i++) {
		rec1();
	}
	return 0;
}
