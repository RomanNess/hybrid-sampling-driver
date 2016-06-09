'''
2015-03
A script to generate compliant input files for the sampling driver prototype.
@author: roman

'''

import subprocess
import sys
import os.path
from collections import defaultdict

# some error handling
if len(sys.argv) < 2:
    exit("Usage: python3 " + sys.argv[0] + " <binaryName> [<unwoundNodes.txt>]")

unwindStepsPerFunction = defaultdict(lambda : 0)
if len(sys.argv) > 2:
    unwNodesFilePath = sys.argv[2]

    unwFile = open(unwNodesFilePath)
    for line in unwFile:
        cols = line.split()
        unwindStepsPerFunction[cols[1]] = cols[0]

    # print(unwindStepsPerFunction)

command = "nm"
targetExePath = sys.argv[1]
if not os.path.isfile(targetExePath):
    exit("File '" + targetExePath + "' was not found.")
else:
    command += " " + targetExePath

# open nm in a subprocess and get output
nmProc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, universal_newlines=True)

# prepare output file
file = open("nm_file", "w")

for line in nmProc.stdout:

    # parse nm output
    cols = line.rstrip().split(" ")
    if len(cols) != 3 or cols[0] == "":
        continue

    address = cols[0]
    name = cols[2]
    unwindSteps = unwindStepsPerFunction[name];

    # write to file
    file.write("{} {} {}\n".format(address, unwindSteps, name))

file.close()  # close output file
nmProc.kill()  # close subprocess

# parse the start and end address for the binary
objdumpCommand = "objdump -d " + sys.argv[1]

objdumpHead = subprocess.Popen(objdumpCommand + " | head", shell=True, stdout=subprocess.PIPE, universal_newlines=True)
for line in objdumpHead.stdout:
    cols = line.rstrip().split(" ")
    if len(cols) == 2:
        start = int(cols[0], 16)
        break
objdumpHead.kill()

# ahahaha ugly hack
objdumpMain = subprocess.Popen(objdumpCommand + ' | grep -A1000 "<main>:"', shell=True, stdout=subprocess.PIPE,
                               universal_newlines=True)
for line in objdumpMain.stdout:
    cols = line.rstrip().split(" ")
    if len(cols) == 2:
        mainFunctionStart = int(cols[0], 16)
        continue
    if len(cols) == 1:
        break
    else:
        mainFunctionEnd = int(cols[2].split(':')[0], 16)
objdumpMain.kill()

objdumpTail = subprocess.Popen(objdumpCommand + " | tail", shell=True, stdout=subprocess.PIPE, universal_newlines=True)
for line in objdumpTail.stdout:
    cols = line.rstrip().split("\t")
    if len(cols) == 3:
        end = int(cols[0][:-1], 16)  # delete last char before parse

file = open("regions_file", "w")
file.write(str(hex(start)) + " " + str(hex(end)) + " " + sys.argv[1] + "\n")
file.write(str(hex(mainFunctionStart)) + " " + str(hex(mainFunctionEnd)) + " main" + "\n")
file.close()
