"""
    This script to gather all Erods-Reny Instances and their optimal value
"""
import os

out_file = "erdos_graphs.txt"
lines = []

for filepath in os.listdir("../../graphs/erdos_graphs"):
    with open(os.path.join("../../graphs/erdos_graphs/", filepath), "r") as instf:
        for line in instf:
            if "% The size of" in line:
                line = "%s,%s\n" % (os.path.join("../../../graphs/erdos_graphs/", filepath), line.strip().split(":")[1].strip().rstrip())
                lines.append(line)
                break

with open(out_file, "w") as outf:
    outf.writelines(lines)
