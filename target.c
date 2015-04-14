
void rec2(int i) {
	i = i * 2;
}

void rec1() {
	for (int i=0; i<1000000; i++) {
		rec2(i);
	}
}

int main() {

	#pragma omp parallel
	{
		rec1();
	}

	return 0;
}
