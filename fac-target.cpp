
#include <iostream>
#include <cstdlib>

template<typename T>
T doCalculation(T val){
	if(val == 0){
		return 1;
	} else {
		return val * doCalculation(val-1);
	}
}


template<typename T>
void calcAndPrintFactorial(T num){
	std::cout << "The result of the factorial was ";
	std::cout << doCalculation(num);
	std::cout << "\n";
}



int main(int argc, char **argv){

	calcAndPrintFactorial(std::atoi(argv[1]));

	return 0;
}
