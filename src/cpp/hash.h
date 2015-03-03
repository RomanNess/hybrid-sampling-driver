
#ifndef SRC_CPP_HASH_H_
#define SRC_CPP_HASH_H_


typedef unsigned int key_type;


#ifdef __cplusplus
extern "C" {
#endif

void put(key_type key, int unwindStep, char* name);
int getUnwindSteps(key_type key);
const char* getName(key_type key);
void parse(char* filename);
void dump();

void parseRegions(char* filename, unsigned int* start, unsigned int* end);

#ifdef __cplusplus
}
#endif


#endif /* SRC_CPP_HASH_H_ */
