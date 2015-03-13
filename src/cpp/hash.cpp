
#include <map>
#include <stdio.h>

#include <fstream>
#include <string>
#include <err.h>
#include <stdlib.h> // strtol
#include <vector>

#include "hash.h"

struct FuncMap {

	std::map<key_type, int> unwindSteps;
	std::map<key_type, std::string> names;

	unsigned long regionStart;
	unsigned long regionEnd;

} FuncMap;

extern "C" {
	void put(key_type key, int unwindStep, char* name) {
		FuncMap.unwindSteps[key] = unwindStep;
		FuncMap.names[key] = std::string(name);
	}

	unsigned int getFunctionStart(key_type address) {

		if (address < FuncMap.regionStart || address > FuncMap.regionEnd) {
			return 0;	// not in interesting region
		}

		auto it = FuncMap.unwindSteps.lower_bound(address);
		--it;
		return (*it).first;
	}

	int getUnwindSteps(key_type key) {
		if (FuncMap.unwindSteps.find(key) != FuncMap.unwindSteps.end()) {
			return FuncMap.unwindSteps[key];
		}
		return -1;
	}

	const char* getName(key_type key) {
		if (FuncMap.names.find(key) != FuncMap.names.end()) {
			return FuncMap.names[key].c_str();
		}
		return (const char*) "n/a";
	}

	/* probably the ugliest possible way to implement this */
	void parse(char* filename) {
		// TODO error handling

		std::ifstream inFile(filename);
		std::string line;

		while (std::getline(inFile, line)) {
			int key = (int) strtol(line.c_str(), NULL, 16);
			char delimiter = ' ';
			size_t secondColStart = line.find(delimiter) + 1;
			size_t secondDelimiter = line.find(delimiter, secondColStart);
			std::string name = line.substr(secondColStart, secondDelimiter-secondColStart);

			int amount = (int) strtol(line.substr(secondDelimiter).c_str(), NULL, 10);

//			printf("%x|%s|%i\n", key, name.c_str(), amount);

			FuncMap.names[key] = name.c_str();
			FuncMap.unwindSteps[key] = amount;
		}

	}


	void dump() {
		for (auto elem : FuncMap.names) {
			printf("%x -> %s \n", elem.first, elem.second.c_str());
		}
	}

	void parseRegions(char* filename, unsigned long* start, unsigned long* end) {
		std::ifstream inFile(filename);

		inFile >> std::hex >> *start;
		inFile >> std::hex >> *end;

		FuncMap.regionStart = *start;
		FuncMap.regionEnd = *end;

	}

}

