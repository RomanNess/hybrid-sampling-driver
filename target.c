
void bar(int i) {
	i = i * 2;
}

void foo() {
	for (int i=0; i<1000000; i++) {
		bar(i);
	}
}

int main() {

	#pragma omp parallel
	{
		foo();
	}

	return 0;
}
