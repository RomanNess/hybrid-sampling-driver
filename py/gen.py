'''
2015-02
A script to generate compliant input files for the sampling driver prototype.
@author: roman

'''

import subprocess
import sys
import os.path

# some error handling
if len(sys.argv) < 2:
	exit("Usage: python3 " + sys.argv[0] + " <binaryName>")
binaryName = sys.argv[1]
if not os.path.isfile(binaryName):
	exit("File '" + binaryName + "' was not found.")

# open nm in a subprocess and get output
process = subprocess.Popen("nm target.exe", shell=True, stdout=subprocess.PIPE, universal_newlines=True)

# prepare output file
file = open("nm_file", "w") 

for line in process.stdout:
	
	# parse nm output
	cols = line.rstrip().split(" ")
	if cols[0] == "":
		continue

	# write to file
	file.write(cols[0] + " " + cols[2] + " 1\n")

file.close()
# close subprocess
process.kill()
