#ifndef SRC_CPP_HASH_H_
#define SRC_CPP_HASH_H_

typedef unsigned long key_type;

#ifdef __cplusplus
extern "C" {
#endif

void put(key_type key, int unwindStep, char* name);
int getUnwindSteps(key_type key);
key_type getFunctionStart(key_type key);
const char* getName(key_type key);
void parseFunctions(char* filename);
void dump();

void dumpMemoryMapping();

void parseRegions(char* filename, unsigned long* start, unsigned long* end);

#ifdef __cplusplus
}
#endif

#endif /* SRC_CPP_HASH_H_ */
