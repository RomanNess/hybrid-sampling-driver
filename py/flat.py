import bisect
import sys

function_bounds = []
address_to_fname = {}

fname_to_sample = {}
unknown_samples = 0
overall_samples = 0


def get_function_name_for_address(address):
    if address < target_region_start or address > target_region_end:
        return

    index = bisect.bisect_left(function_bounds, address)
    # print("{} -> {} -> {}".format(index, function_bounds[index - 1], function_names[function_bounds[index - 1]]))
    return address_to_fname[function_bounds[index - 1]]


def dump_invocations():
    functions_with_no_samples = 0
    known_samples = overall_samples - unknown_samples

    file = open(base + "sample_percent.txt", "w")

    for name, samples in fname_to_sample.items():
        sample_percent = 10000. * samples / known_samples

        file.write("{} % ")
        if samples > 1:
            print("{} -- {} [per 10000] -- {}".format(samples, sample_percent, name))

        if samples <= 1:
            functions_with_no_samples += 1
    print("{} functions with no samples".format(functions_with_no_samples))


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
    address_to_fname[address] = name
# prepare bounds
function_bounds.sort()
target_region_start = function_bounds[0]
target_region_end = function_bounds[-1]

# print(function_names)
# print(function_bounds)

for name in address_to_fname.values():
    fname_to_sample[name] = 1

# parse flat profile
file = open(base + "flat_profile", "r")
for line in file:
    address = int(line, 16)
    name = get_function_name_for_address(address)

    if name:
        fname_to_sample[name] += 1
    else:
        unknown_samples += 1
    overall_samples += 1
file.close()

print("{} overall invocations ({} in unknown functions.)".format(overall_samples, unknown_samples))
# print(function_invocations)
dump_invocations()
