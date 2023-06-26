"""
    Translate graphs in gml format to dimacs format
"""
import os.path
from enum import Enum
import sys


class ReadStatus(Enum):
    GRAPH = 1
    NODE = 2
    EDGE = 3


def gml_to_dimacs(gml_filename, out_dir):
    out_lines = []

    nodes = []

    edges = []

    with open(gml_filename, "r") as gml_file:

        read_status = []

        node = 0
        source = 0
        target = 0

        for line in gml_file:
            if line[0] == "#":
                line = "%" + line[1:]
                out_lines.append(line)
            elif "graph [" in line and len(read_status) == 0:
                read_status.append(ReadStatus.GRAPH)
            elif "node [" in line and read_status[-1] == ReadStatus.GRAPH:
                read_status.append(ReadStatus.NODE)
            elif "edge [" in line and read_status[-1] == ReadStatus.GRAPH:
                read_status.append(ReadStatus.EDGE)
            elif "]" in line:
                if read_status[-1] == ReadStatus.NODE:
                    nodes.append(int(node))
                elif read_status[-1] == ReadStatus.EDGE:
                    edges.append((source, target))
                elif read_status[-1] == ReadStatus.GRAPH:
                    break  # finished

                read_status.pop(-1)
                node = 0
                source = 0
                target = 0
            elif "id" in line and read_status[-1] == ReadStatus.NODE:
                node = int(line.strip().split(" ")[1].rstrip())
            elif "source" in line and read_status[-1] == ReadStatus.EDGE:
                source = int(line.strip().split(" ")[1].rstrip())
            elif "target" in line and read_status[-1] == ReadStatus.EDGE:
                target = int(line.strip().split(" ")[1].rstrip())

    if len(nodes) - 1 != int(sorted(nodes)[-1]):
        print("Nodes have not continuous ids")
        return

    adjs = [[] for _ in range(len(nodes))]

    for (s, t) in edges:
        adjs[int(s)].append(int(t) + 1)
        adjs[int(t)].append(int(s) + 1)

    out_lines.append("%s %s\n" % (len(nodes), len(edges)))

    for adj in adjs:
        out_lines.append(" ".join([str(t) for t in adj]) + "\n")

    with open(os.path.join(out_dir, os.path.basename(gml_filename).split(".")[0] + ".graph"), "w") as graph_file:
        graph_file.writelines(out_lines)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 graph_gml_to_dimacs instance.gml out_dir")
    else:
        gml_to_dimacs(sys.argv[1], sys.argv[2])
