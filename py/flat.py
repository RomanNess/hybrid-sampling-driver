
import bisect
import sys

target_region_start, target_region_end = 0, 0
function_bounds = []
function_names = {}


def func(address):

    if address < target_region_start or address > target_region_end:
        return

    index = bisect.bisect_left(function_bounds, address)
    print("{} -> {} -> {}".format(index, function_bounds[index - 1], function_names[function_bounds[index - 1]]))


base = "/home/roman/repos/libsampling/"

file = open(base + "nm_file", "r")
for line in file:
    print(line)

file = open(base + "flat_profile", "r")
for line in file:
    address = int(line, 16)
    print(address)