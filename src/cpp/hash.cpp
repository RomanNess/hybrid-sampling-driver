
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

#define DEBUG 1

struct FuncMap {

	std::map<key_type, int> unwindSteps;
	std::map<key_type, std::string> names;

	unsigned long targetRegionStart;
	unsigned long targetRegionEnd;

	unsigned long driverRegionStart;
	unsigned long driverRegionEnd;

} FuncMap;

extern "C" {
	void put(key_type key, int unwindStep, char* name) {
		FuncMap.unwindSteps[key] = unwindStep;
		FuncMap.names[key] = std::string(name);
	}

	key_type getFunctionStartAddress(key_type address) {

		if (address < FuncMap.targetRegionStart || address > FuncMap.targetRegionEnd) {
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
		key = getFunctionStartAddress(key);
		if (FuncMap.names.find(key) != FuncMap.names.end()) {
			return FuncMap.names[key].c_str();
		}
		return (const char*) "n/a";
	}

	void parseFunctions(char* filename) {

		std::ifstream inFile(filename);
		if (!inFile.is_open()) {
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

	void parseRegions(char* filename, unsigned long* start, unsigned long* end,
			unsigned long* mainStart, unsigned long* mainEnd) {

		std::ifstream inFile(filename);
		if (!inFile.is_open()) {
			errx(1, "Error: File \"%s\" not found.", filename);
		}

		inFile >> std::hex >> *start >> *end;

		FuncMap.targetRegionStart = *start;
		FuncMap.targetRegionEnd = *end;

		inFile.ignore(256, '\n'); // get next line
		inFile >> std::hex >> *mainStart >> *mainEnd;

#if DEBUG
		std::cout << "Parsed target regions: " << std::hex << FuncMap.targetRegionStart
			<< " - " << FuncMap.targetRegionEnd
			<< " (mainFunction: " << *mainStart << " - " << *mainEnd << " )" << std::endl;
#endif
	}

	/* note that executing "cp" in the shell would create a new process
	 * and trigger libmonitors callbacks */
	void dumpMemoryMapping(unsigned long* start, unsigned long* end) {
		int pid = getpid();
		std::string srcFile = "/proc/" + std::to_string(pid) +  "/maps";
		std::string destFile = "map_file";

		std::ifstream src(srcFile);
		std::ofstream dest(destFile);

		if (src.fail()) {
			std::cout << "Error: Could not read map from " << srcFile << std::endl;
		}

		std::string stringBuf;
		std::string line;
		while (std::getline(src, line)) {
			if (line.find("libsampling/lib/libsampling") != std::string::npos) {
				// found
//				std::cout << "line: " << line << std::endl;

				std::stringstream lineStream(line);
				lineStream >> std::hex >> *start;
				lineStream.ignore(1);
				lineStream >> std::hex >> *end;

				break;
			}
			dest << line << std::endl;
		}

		FuncMap.driverRegionStart = *start;
		FuncMap.driverRegionEnd = *end;

		src.close();
		dest.close();
#if DEBUG
		std::cout << "Parsed driver regions: " << std::hex << FuncMap.driverRegionStart
			<< " - " << FuncMap.driverRegionEnd << std::endl;
#endif
	}
}

