
#include <map>
#include <stdio.h>

#include <fstream>
#include <sstream>

#include <string>
#include <err.h>
#include <unistd.h>	// for getpid
#include <stdlib.h> // strtol
#include <vector>
#include <iostream> // XXX remove

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

	key_type getFunctionStartAddress(key_type address) {

		if (address < FuncMap.regionStart || address > FuncMap.regionEnd) {
			return address;	// XXX not in interesting region
		}

		if (FuncMap.unwindSteps.find(address) != FuncMap.unwindSteps.end()) {
			return address;	// no exact function address yet
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

	void parseFunctions(char* filename) {

		std::ifstream inFile(filename);
		if (inFile == NULL) {
			errx(1, "Error: File \"%s\" not found.", filename);
		}

		std::string line;
		while (std::getline(inFile, line)) {
			int key = (int) strtol(line.c_str(), NULL, 16);
			char delimiter = ' ';
			size_t secondColStart = line.find(delimiter) + 1;
			size_t secondDelimiter = line.find(delimiter, secondColStart);
			std::string name = line.substr(secondColStart, secondDelimiter-secondColStart);

			int amount = (int) strtol(line.substr(secondDelimiter).c_str(), NULL, 10);

			FuncMap.names[key] = name.c_str();
			FuncMap.unwindSteps[key] = amount;
		}

	}


	void dump() {
		for (auto elem : FuncMap.names) {
			printf("%lx -> %s \n", elem.first, elem.second.c_str());
		}
	}

	void parseRegions(char* filename, unsigned long* start, unsigned long* end) {
		std::ifstream inFile(filename);
		if (inFile == NULL) {
			errx(1, "Error: File \"%s\" not found.", filename);
		}

		inFile >> std::hex >> *start;
		inFile >> std::hex >> *end;

		FuncMap.regionStart = *start;
		FuncMap.regionEnd = *end;

	}

	/* note that executing "cp" in the shell would create a new process
	 * and trigger libmonitors callbacks */
	void dumpMemoryMapping() {
		int pid = getpid();
		std::string srcFile = "/proc/" + std::to_string(pid) +  "/maps";
		std::string destFile = "map_file";

		std::ifstream src(srcFile);
		std::ofstream dest(destFile);

		if (src.fail()) {
			std::cout << "Error: Could not read map from " << srcFile << std::endl;
		}

		dest << src.rdbuf();

		src.close();
		dest.close();
	}
}

