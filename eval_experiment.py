"""
    A small python script to parse the logs from the algorithms
"""
import csv
import os
import sys
from dataclasses import dataclass
from enum import Enum

time_limit = 36000000  # ms


def sec_to_ms(time):
    return time * 1000


class TimeStatus(Enum):
    GOOD = "good"
    TIMEOUT = "timeout"
    MEMOUT = "memout"
    NONE = "none"


class SolStatus(Enum):
    FOUND = "found"
    NONE = "none"


@dataclass
class Time:
    t: float
    status: TimeStatus


@dataclass
class Solution:
    sol: int
    status: SolStatus


@dataclass
class Result:
    instance_name: str
    time: Time
    solution: Solution
    seed: int
    kernel_nodes: int  # number of nodes after 2ps reductions
    kernel_edges: int  # number of nodes after 2ps reductions
    nodes: int  # number of nodes
    offset: int
    kernel_size: int
    time_kamis: float
    time_transform: float


def get_data_m2s_bnr(file):
    name = ""
    solution = Solution(0, SolStatus.NONE)
    seed = int(os.path.basename(file)[1:].split("_s")[1][0])
    time = Time(0, TimeStatus.MEMOUT)
    kernel_nodes = 0
    kernel_edges = 0
    nodes = 0
    offset = 0
    kernel_size = 0
    time_kamis = 0.0
    time_transform = 0.0

    if not os.path.exists(file):
        return None

    with open(file, "r") as result_f:
        for line in result_f:
            if "Transformed G Nodes" in line:
                kernel_nodes = int(line.split(":")[1].rstrip().strip())
            if "Transformed G Edges" in line:
                kernel_edges = int(line.split(":")[1].rstrip().strip())
            elif "-Nodes" in line:
                nodes = int(line.split(":")[1].rstrip().strip())
            elif "Size" in line:
                solution.sol = int(line.split(":")[1].rstrip().strip())
                solution.status = SolStatus.FOUND
            elif "Filename" in line:
                name = line.split(":")[1].rstrip().strip()
            elif "Time found" in line:
                time.t = sec_to_ms(float(line.split(":")[1].rstrip().strip()))
                if time.t > time_limit:
                    time.status = TimeStatus.TIMEOUT
                else:
                    time.status = TimeStatus.GOOD
            elif "Transformation time" in line or "Reduction time" in line:
                time_transform += sec_to_ms(float(line.split(":")[1].rstrip().strip()))
            elif "Offset" in line:
                offset = float(line.split(":")[1].rstrip().strip())
            elif "[" in line and "]" in line:
                kernel_size = int(line.split(" [")[0])
                time_kamis = sec_to_ms(float(line.split(" [")[1].split("]")[0]))

    if time.status != TimeStatus.MEMOUT and time_kamis == 0.0 and kernel_nodes > 0:
        time_kamis = time.t - time_transform

    return Result(name, time, solution, seed, kernel_nodes, kernel_edges, nodes, offset, kernel_size, time_kamis,
                  time_transform)
    
def get_data_m2s_heuristic(file):
    name = ""
    solution = Solution(0, SolStatus.NONE)
    seed = int(os.path.basename(file)[1:].split("_s")[1][0])
    time = Time(0, TimeStatus.MEMOUT)
    kernel_nodes = 0
    kernel_edges = 0
    nodes = 0
    offset = 0
    kernel_size = 0
    time_kamis = 0.0
    time_transform = 0.0

    if not os.path.exists(file):
        return None

    with open(file, "r") as result_f:
        for line in result_f:
            if "Transformed G Nodes" in line:
                kernel_nodes = int(line.split(":")[1].rstrip().strip())
            if "Transformed G Edges" in line:
                kernel_edges = int(line.split(":")[1].rstrip().strip())
            elif "-Nodes" in line:
                nodes = int(line.split(":")[1].rstrip().strip())
            elif "Size:" in line:
                if not "[" in line:
                    solution.sol = int(line.split(":")[1].rstrip().strip())
                    solution.status = SolStatus.FOUND
            elif "Filename" in line:
                name = line.split(":")[1].rstrip().strip()
            elif "Time found" in line:
                time.t = sec_to_ms(float(line.split(":")[1].rstrip().strip()))
                if time.t > time_limit:
                    time.status = TimeStatus.TIMEOUT
                else:
                    time.status = TimeStatus.GOOD
            elif "Transformation time" in line or "Reduction time" in line:
                time_transform += sec_to_ms(float(line.split(":")[1].rstrip().strip()))
            elif "Offset" in line:
                offset = float(line.split(":")[1].rstrip().strip())

    return Result(name, time, solution, seed, kernel_nodes, kernel_edges, nodes, offset, kernel_size,time_transform,
                  time_transform)



def get_data_apx2p(file):
    name = ""
    solution = Solution(0, SolStatus.NONE)
    seed = int(os.path.basename(file)[1:].split("_s")[1][0])
    time = Time(0, TimeStatus.MEMOUT)

    if not os.path.exists(file):
        return None

    with open(file, "r") as result_f:
        for line in result_f:
            if "Checked solution size:" in line:
                solution.sol = int(line.split(":")[1].rstrip().strip())
                solution.status = SolStatus.FOUND
            elif "Openning" in line:
                name = line.split("/")[-1].rstrip().strip()
            elif "Time:" in line:
                time.t = sec_to_ms(float(line.split(":")[1].rstrip().strip()))
                if time.t > time_limit:
                    time.status = TimeStatus.TIMEOUT
                else:
                    time.status = TimeStatus.GOOD

    return Result(name, time, solution, seed, 0, 0, 0, 0, 0, 0, 0)


def get_data_genetic_algo(file):
    results = []

    if not os.path.exists(file) or "results_" not in file:
        return results

    with open(file, "r") as result_f:
        reader = csv.DictReader(result_f, delimiter=",")
        for row in reader:
            name = row["Graph"]
            size = Solution(int(float(row["GA_withImp"])), SolStatus.FOUND)
            seed = int(row["Seed"])
            time = Time(sec_to_ms(float(row["Init_Time"]) + float(row["Solve_Time"])), TimeStatus.GOOD, )
            if time.t > time_limit:
                time.status = TimeStatus.TIMEOUT

            results.append(Result(name, time, size, seed, 0, 0, 0, 0, 0, 0, 0, ))

    if len(results) == 0:
        # timeout/memout without finding any sol.
        with open(file.replace("results_", "gene2pack_").replace("_wake.csv", ".log"), "r") as log_f:
            memout = False
            for line in log_f:
                if "Out of memory" in line:
                    memout = True
                    break
            results.append(Result(file.split("results_")[1].split("_wake.csv")[0],
                                  Time(time_limit + 1, TimeStatus.TIMEOUT) if not memout else Time(0,
                                                                                                   TimeStatus.MEMOUT),
                                  Solution(0, SolStatus.NONE), None, 0, 0, 0, 0, 0, 0, 0, ))

    return results


def print_2pack(res):
    print(" ".join([str(res.solution.sol) if res.solution.status == SolStatus.FOUND else str(res.solution.status),
                    str(round(res.time_transform + res.time_kamis, 2)),
                    str(round(res.time.t, 2)) if res.time.status == TimeStatus.GOOD else str(res.time.status),
                    str(res.kernel_nodes), str(res.kernel_edges)]))


def print_red2pack(res):
    print(" ".join([str(res.solution.sol) if res.solution.status == SolStatus.FOUND else str(res.solution.status),
                    str(round(res.time_transform + res.time_kamis, 2)),
                    str(round(res.time.t, 2)) if res.time.status == TimeStatus.GOOD else str(res.time.status),
                    str(res.kernel_nodes), str(res.kernel_edges)]))

def print_red2pack_heuristic(res):
    print(" ".join([str(res.solution.sol), str(round(res.time.t, 2)),
                    str(res.kernel_nodes), str(res.kernel_edges)]))

def print_gen2pack(res):
    print(" ".join([str(res.solution.sol) if res.solution.status == SolStatus.FOUND else str(res.solution.status),
                    str(round(res.time.t, 2)) if res.time.status == TimeStatus.GOOD else str(res.time.status)]))


def print_apx2p(res):
    print(" ".join([str(res.solution.sol) if res.solution.status == SolStatus.FOUND else str(res.solution.status),
                    str(round(res.time.t, 2)) if res.time.status == TimeStatus.GOOD else str(res.time.status)]))


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python3 %s <2pack:red2pack:gen2pack:apx2p> <log_file> <time_limit_in_sec>" % sys.argv[0])
    else:
        time_limit = sec_to_ms(int(sys.argv[3]))
        if sys.argv[1] == "2pack":
            res = get_data_m2s_bnr(sys.argv[2])
            print_2pack(res)
        elif sys.argv[1] == "red2pack":
            res = get_data_m2s_bnr(sys.argv[2])
            print_red2pack(res)
        elif sys.argv[1] == "red2pack_heuristic":
            res = get_data_m2s_heuristic(sys.argv[2])
            print_red2pack_heuristic(res)
        elif sys.argv[1] == "gen2pack":
            res = get_data_genetic_algo(sys.argv[2])
            # For the example experiment we only compute one result
            res = res[0]
            print_gen2pack(res)
        elif sys.argv[1] == "apx-2p":
            res = get_data_apx2p(sys.argv[2])
            print_apx2p(res)
