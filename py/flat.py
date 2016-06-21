
import bisect
import sys

target_region_start, target_region_end = 0, 0
function_bounds = []
function_names = {}

function_invocations = {}


def get_function_name_for_address(address):

    if address < target_region_start or address > target_region_end:
        return

    index = bisect.bisect_left(function_bounds, address)
    print("{} -> {} -> {}".format(index, function_bounds[index - 1], function_names[function_bounds[index - 1]]))
    return function_names[function_bounds[index - 1]]

if len(sys.argv) > 1:
    base = sys.argv[1] + "/"
else:
    base = "/home/roman/repos/libsampling/"
print("running flat.py in " + base)

# parse nm-file
file = open(base + "nm_file", "r")
for line in file:
    cols = line.split()
    address = int(cols[0], 16)
    name = " ".join(cols[2:])

    function_bounds.append(address)
    function_names[address] = name
# prepare bounds
function_bounds.sort()
target_region_start = function_bounds[0]
target_region_end = function_bounds[-1]

# print(function_names)
# print(function_bounds)

for name in function_names.values():
    function_invocations[name] = 0

# parse flat profile
file = open(base + "flat_profile", "r")
for line in file:
    address = int(line, 16)
    name = get_function_name_for_address(address)
    # print(name)