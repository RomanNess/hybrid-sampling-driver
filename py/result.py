'''
2015-04
A script to parse the micro benchmark results and dump them CSV style
@author: roman

'''


dict = dict()

# parse result
inFile = open("./lsf/OUT.out", "r")
for line in inFile.readlines():
	if "==" in line:
		cols = line.split()
		name = cols[0]
		val = cols[-2]
		
		if not name in dict:
			dict[name] = []
			
		dict[name].append(val)
inFile.close()

#dump
outFile = open("./out", "w")
for key, val in dict.items():
	outFile.write(key + "\t")
	for e in val:
		outFile.write(e + "\t")
	outFile.write("\n")
outFile.close()
