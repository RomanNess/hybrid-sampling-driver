void bar() {
}

void foo() {
	for (int i = 0; i < 10; i++) {
		bar();
	}
}

int main() {

	foo();

	return 0;
}
