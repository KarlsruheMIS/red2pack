import csv
import os.path
from collections import OrderedDict
from dataclasses import dataclass
from statistics import geometric_mean, mean
from typing import List, Callable, Dict
from enum import Enum

from data_table import DataTable, ColGroup, ColHeader, RowGroup, RowHeader


time_limit = 36000000  # ms


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
    size: Solution
    seed: int
    kernel_nodes: int  # number of nodes after 2ps reductions
    kernel_edges: int  # number of nodes after 2ps reductions
    nodes: int  # number of nodes
    offset: int
    kernel_size: int
    time_kamis: float
    time_transform: float


@dataclass
class AlgoResults:
    label: str
    files: List[str]
    agg_cols: bool
    get_data: Callable[[str], List[Result]]

    data: Dict[str, List[Result]]

    def load(self):
        for file in self.files:
            results = self.get_data(file)
            for result in results:
                if result is not None:
                    instance = result.instance_name.split(".")[0]
                    if instance not in self.data.keys():
                        self.data[instance] = []
                    self.data[instance].append(result)


@dataclass
class InstanceGroup:
    key: str
    name: str
    instances: List[str]
    print_agg_row: bool


def get_data_m2s_bnr(file):
    name = ""
    size = Solution(0, SolStatus.NONE)
    seed = int(os.path.basename(file)[1:].split("_")[0])
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
                size.sol = int(line.split(":")[1].rstrip().strip())
                size.status = SolStatus.FOUND
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

    return [
        Result(
            name,
            time,
            size,
            seed,
            kernel_nodes,
            kernel_edges,
            nodes,
            offset,
            kernel_size,
            time_kamis,
            time_transform,
        )
    ]


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
            time = Time(
                sec_to_ms(float(row["Init_Time"]) + float(row["Solve_Time"])),
                TimeStatus.GOOD,
            )
            if time.t > time_limit:
                time.status = TimeStatus.TIMEOUT
            kernel_nodes = 0
            kernel_edges = 0
            nodes = 0

            results.append(
                Result(
                    name,
                    time,
                    size,
                    seed,
                    kernel_nodes,
                    kernel_edges,
                    nodes,
                    0,
                    0,
                    0,
                    0,
                )
            )

    if len(results) == 0:
        # timeout/memout without finding any sol.
        with open(
            file.replace("results_", "").replace("_wake.csv", ".log"), "r"
        ) as log_f:
            memout = False
            for line in log_f:
                if "Out of memory" in line:
                    memout = True
                    break
            results.append(
                Result(
                    file.split("results_")[1].split("_wake.csv")[0],
                    Time(time_limit + 1, TimeStatus.TIMEOUT)
                    if not memout
                    else Time(0, TimeStatus.MEMOUT),
                    Solution(0, SolStatus.NONE),
                    None,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                )
            )

    return results


def get_file_paths(dir_names: List[str]):
    file_paths = []
    for dir_name in dir_names:
        file_paths = file_paths + [
            os.path.join(dir_name, inst_f) for inst_f in os.listdir(dir_name)
        ]

    return file_paths


def print_all():
    m2s_bnr_all_reductions = AlgoResults(
        label="red2pack elaborated",
        files=get_file_paths(["out/social_graphs/all_reductions"]),
        get_data=get_data_m2s_bnr,
        data={},
    )

    m2s_bnr_all_reductions.load()

    for instance, results in m2s_bnr_all_reductions.data.items():
        check = True
        for r in results:
            if r.timeout:
                print("%s timeout" % instance)
                check = False
                break
            if r.size == 0:
                print("%s sol not found" % instance)
                check = False
                break
        if check:
            print(
                "%s, %s, %s, %s, %s"
                % (
                    instance,
                    results[0].size,
                    geometric_mean([r.time for r in results]),
                    results[0].nodes,
                    mean([r.kernel_nodes for r in results]),
                )
            )


def print_erdos_gen2pack():
    gen2pack = AlgoResults(
        label="gen2pack",
        files=get_file_paths(["../../2-packing-Set/out"]),
        get_data=get_data_genetic_algo,
        data={},
        agg_cols=False,
    )

    gen2pack.load()

    count_optimal_runs = 0
    experiments = 0
    count_optimal_instances = 0
    for instance, results in gen2pack.data.items():
        opt_size = 0

        with open(
            os.path.join("graphs/erdos_graphs_gml/", instance + ".gml"), "r"
        ) as instance_fd:
            for line in instance_fd:
                if "maximum 2-packing set" in line:
                    opt_size = int(line.split(":")[1].strip())
                    break
        opt = False
        for r in results:
            if r.size == opt_size:
                count_optimal_runs += 1
                opt = True
        # print(results)
        experiments += len(results)
        if opt:
            count_optimal_instances += 1

    print(
        "Gen2pack found in  %s/%s runs an optimal solution and for %s/%s instances at least once an optimal solution."
        % (count_optimal_runs, experiments, count_optimal_instances, len(gen2pack.data))
    )


def print_time(time, digits=2):
    if time is None:
        print("WARNING: time should not be None. Use Time(0, TimeStatus.NONE) instead!")
        return "-"  # no runs
    if time.status == TimeStatus.MEMOUT:
        return "m.o."
    if time.status == TimeStatus.TIMEOUT:
        return "t.o."
    return "\\numprint{" + ("{:.%sf}" % digits).format(round(time.t, digits)) + "}"


def print_size(size, digits=0):
    if size is None:
        print(
            "WARNING: size should not be None. Use Solution(0, SolStatus.NONE) instead!"
        )
        return "-"

    if size.status == SolStatus.NONE:
        return "-"

    return "\\numprint{" + ("{:.%sf}" % digits).format(round(size.sol, digits)) + "}"


def print_size_approx(size, digits=2):
    return "\\numprint{" + ("{:.%sf}" % digits).format(round(size, digits)) + "}"


def agg_size(sizes: List[Solution]):
    feasible = [s.sol for s in sizes if s.status == SolStatus.FOUND]

    if len(feasible) == 0:
        return Solution(0, SolStatus.NONE)

    if 0 in feasible:
        print("ERROR: 0 in feasible solution sizes.")
        quit()

    return Solution(geometric_mean(feasible), SolStatus.FOUND)


def agg_time(times: List[Time]):
    memout = True
    for t in times:
        if t.status != TimeStatus.MEMOUT:
            memout = False
            break
    if memout:
        return Time(0, TimeStatus.MEMOUT)
    new_t = geometric_mean([t.t for t in times if t.status != TimeStatus.MEMOUT])
    status = TimeStatus.GOOD
    if new_t > time_limit:
        status = TimeStatus.TIMEOUT
    return Time(new_t, status)


def agg_time_mean(times: List[Time]):
    memout = True
    for t in times:
        if t.status != TimeStatus.MEMOUT:
            memout = False
            break
    if memout:
        return Time(0, TimeStatus.MEMOUT)
    new_t = mean([t.t for t in times])
    status = TimeStatus.GOOD
    if new_t > time_limit:
        status = TimeStatus.TIMEOUT
    return Time(new_t, status)


def get_size(data):
    if data.size.sol > 0:
        return data.size
    return Solution(data.kernel_size + data.offset, SolStatus.FOUND)


def sec_to_ms(time):
    if time == -1:
        return time
    else:
        return time * 1000


def print_result_time_performance(
    algo_results: List[AlgoResults], instances_groups: List[InstanceGroup]
):
    for algo_result in algo_results:
        algo_result.load()

    cols = []
    for algo_result in algo_results:
        if algo_result.label != "gen2pack":
            cols.append(
                ColGroup(
                    algo_result.label,
                    algo_result.label,
                    [
                        ColHeader(
                            "$|S|$",
                            "size",
                            lambda sols: agg_size(sols),
                            lambda data, filename: get_size(data),
                            print_size,
                            False,
                            False,
                        ),
                        ColHeader(
                            "$t_p$ [ms]",
                            "time-proof",
                            lambda times: agg_time(times),
                            lambda data, filename: data.time,  # s to ms
                            lambda x: print_time(x),
                            algo_result.agg_cols,
                            False,
                        ),
                        ColHeader(
                            "$t$ [ms]",
                            "time",
                            lambda times: agg_time(times),
                            lambda data, filename: Time(
                                data.time_transform + data.time_kamis, TimeStatus.GOOD
                            ),  # s to mas
                            lambda x: print_time(x),
                            algo_result.agg_cols,
                            False,
                        ),
                    ],
                )
            )
        else:
            cols.append(
                ColGroup(
                    algo_result.label,
                    algo_result.label,
                    [
                        ColHeader(
                            "$|S|$",
                            "size",
                            lambda sols: agg_size(sols),
                            lambda data, filename: data.size,
                            print_size,
                            algo_result.agg_cols,
                        ),
                        ColHeader(
                            "$t$ [ms]",
                            "time",
                            lambda times: agg_time(times),
                            lambda data, filename: data.time,  # s to ms
                            lambda x: print_time(x),
                            algo_result.agg_cols,
                            False,
                        ),
                    ],
                )
            )

    cols = sorted(cols, key=lambda x: x.name)

    rows = [
        RowGroup(
            group.name,
            group.key,
            [
                RowHeader(inst.replace("_", "\\textunderscore "), inst)
                for inst in group.instances
            ],
            group.print_agg_row,
        )
        for group in instances_groups
    ]

    table = DataTable(cols, rows)

    inst_to_group = {
        inst: group.key for group in instances_groups for inst in group.instances
    }

    for algo_result in algo_results:
        for inst, result in algo_result.data.items():
            if inst in inst_to_group.keys():
                group = inst_to_group[inst]
                for r in result:
                    table.load_data_in_cell(r, "", group, inst, algo_result.label)

    def found_optimal_sol(cols_in_table, row_key, get_col_val):
        return False

    table.finish_loading(OrderedDict({}), found_optimal_sol)

    labels = sorted([algo_result.label for algo_result in algo_results])
    y_label_time = "$\\\\#\\\\{\\\\mathrm{instances}\\\\leq \\\\tau\\\\cdot\\\\mathrm{fastest}\\\\}/\\\\#\\\\mathrm{instances}~[\\\\%]$"
    y_label_sol = "$\\\\#\\\\{\\\\mathrm{instances}\\\\geq \\\\tau\\\\cdot\\\\mathrm{best}\\\\}/\\\\#\\\\mathrm{instances}~[\\\\%]$"
    table.print_performance(
        "Solution Quality %s" % instances_groups[0].name,
        "plots/performance_sol_%s" % instances_groups[0].name,
        labels,
        "size",
        max,
        instances_groups[0].key,
        1.0,
        0.8,
        y_label=y_label_sol,
        exclude=lambda x: x.status == SolStatus.NONE, # exclude results from set of solutions
        get_val=lambda val: val.sol,
    )
    table.print_performance(
        "Time %s" % instances_groups[0].name,
        "plots/performance_time_%s" % instances_groups[0].name,
        labels,
        "time",
        min,
        instances_groups[0].key,
        1.0,
        2.0,
        y_label=y_label_time,
        exclude=lambda x: x.status == TimeStatus.MEMOUT, # exclude results that produced memout from set of solutions
        get_val=lambda val: val.t,
    )
    table.print_performance(
        "Time %s (Log. Scale)" % instances_groups[0].name,
        "plots/performance_time_%s_log" % instances_groups[0].name,
        labels,
        "time",
        min,
        instances_groups[0].key,
        1.0,
        1000000.0,
        ticks=1.0,
        y_label=y_label_time,
        x_log_scale=True,
        exclude=lambda x: x.status == TimeStatus.MEMOUT, # exclude results that produced memout from set of solutions
        get_val=lambda val: val.t,
    )


def print_result_sol_time_table(
    algo_results: List[AlgoResults], instances_groups: List[InstanceGroup], out_filename
):
    for algo_result in algo_results:
        algo_result.load()

    cols = []
    for algo_result in algo_results:
        if algo_result.label != "gen2pack":
            cols.append(
                ColGroup(
                    algo_result.label,
                    algo_result.label,
                    [
                        ColHeader(
                            "$|S|$",
                            "size",
                            lambda sols: agg_size(sols),
                            lambda data, filename: get_size(data),
                            lambda vals: print_size(vals),
                            algo_result.agg_cols,
                            False,
                        ),
                        ColHeader(
                            "$t$ [ms]",
                            "time",
                            lambda times: agg_time(times),
                            lambda data, filename: Time(
                                data.time_transform + data.time_kamis, TimeStatus.GOOD
                            ),  # s to ms
                            lambda x: print_time(x),
                            algo_result.agg_cols,
                            False,
                        ),
                        ColHeader(
                            "$t_p$ [ms]",
                            "time-proof",
                            lambda times: agg_time(times),
                            lambda data, filename: data.time,  # s to ms
                            lambda x: print_time(x),
                            algo_result.agg_cols and False,
                            False,
                        ),
                    ],
                )
            )
        else:
            cols.append(
                ColGroup(
                    algo_result.label,
                    algo_result.label,
                    [
                        ColHeader(
                            "$|S|$",
                            "size",
                            lambda sols: agg_size(sols),
                            lambda data, filename: data.size,
                            lambda vals: print_size(vals),
                            algo_result.agg_cols,
                            False,
                        ),
                        ColHeader(
                            "$t$ [ms]",
                            "time",
                            lambda times: agg_time(times),
                            lambda data, filename: data.time,  # s to ms
                            lambda x: print_time(x),
                            algo_result.agg_cols,
                            False,
                        ),
                    ],
                )
            )

    cols = sorted(cols, key=lambda x: x.name)

    rows = [
        RowGroup(
            group.name,
            group.key,
            [
                RowHeader(inst.replace("_", "\\textunderscore "), inst)
                for inst in group.instances
            ],
            group.print_agg_row,
        )
        for group in instances_groups
    ]

    table = DataTable(cols, rows)

    inst_to_group = {
        inst: group.key for group in instances_groups for inst in group.instances
    }

    for algo_result in algo_results:
        for inst, result in algo_result.data.items():
            if inst in inst_to_group.keys():
                group = inst_to_group[inst]
                for r in result:
                    table.load_data_in_cell(r, "", group, inst, algo_result.label)

    def found_optimal_sol(cols_in_table, row_key, get_col_val):
        for algo_result in algo_results:
            if algo_result.label != "gen2pack":
                size = get_col_val(algo_result.label, "size")
                time = get_col_val(algo_result.label, "time-proof")

                if (
                    time is not None
                    and time.status == TimeStatus.GOOD
                    and size.status == SolStatus.FOUND
                ):
                    return True

        return False

    table.finish_loading(
        OrderedDict(
            {
                "size": (lambda vals: max([v.sol for v in vals]), []),
                "time": (lambda vals: min([t.t for t in vals]), ["size"]),
                "time-proof": (lambda vals: min([t.t for t in vals]), ["size"]),
            }
        ),
        found_optimal_sol,
    )
    table.print(out_filename)


def print_result_kernel_table(
    algo_results: List[AlgoResults], instances_groups: List[InstanceGroup], out_filename
):
    for algo_result in algo_results:
        algo_result.load()

    cols = [
        ColGroup(
            algo_result.label,
            algo_result.label,
            [
                ColHeader(
                    "$n(\\tilde{G})$",
                    "transformed",
                    lambda nodes: mean(nodes) if len(nodes) != 0 else 0,
                    lambda data, filename: data.kernel_nodes,
                    round,
                    True,
                ),
                ColHeader(
                    "$m(\\tilde{G})$",
                    "transformed-edges",
                    lambda edges: mean(edges) if len(edges) != 0 else 0,
                    lambda data, filename: data.kernel_edges,
                    round,
                    True,
                ),
                ColHeader(
                    "$t_{\\texttt{t}}$~[ms]",
                    "transform-time",
                    lambda tt: agg_time_mean(tt),
                    lambda data, filename: Time(
                        data.time_transform, TimeStatus.GOOD
                    ),
                    lambda x: print_time(x),
                    True,
                    False,
                ),
                ColHeader(
                    "$t_{\\texttt{solve}}~[ms]$",
                    "kamis-time",
                    lambda kt: agg_time_mean(kt),
                    lambda data, filename: Time(
                        data.time_kamis, TimeStatus.GOOD
                    ),
                    lambda x: print_time(x),
                    True,
                    False,
                ),
            ],
        )
        for algo_result in algo_results
        if algo_result.label != "2pack"
    ]

    if "2pack" in [algo_result.label for algo_result in algo_results]:
        cols.append(
            ColGroup(
                "2pack",
                "2pack",
                [
                    ColHeader(
                        "$m(\\tilde{G})$",
                        "transformed-edges",
                        lambda edges: mean(edges) if len(edges) != 0 else 0,
                        lambda data, filename: data.kernel_edges,
                        round,
                        True,
                    ),
                    ColHeader(
                        "$t_{\\texttt{t}}$~[ms]",
                        "transform-time",
                        lambda tt: agg_time_mean(tt),
                        lambda data, filename: Time(
                            data.time_transform, TimeStatus.GOOD
                        ),
                        lambda x: print_time(x),
                        True,
                        False,
                    ),
                    ColHeader(
                        "$t_{\\texttt{solve}}~[ms]$",
                        "kamis-time",
                        lambda kt: agg_time_mean(kt),
                        lambda data, filename: Time(
                            data.time_kamis, TimeStatus.GOOD
                        ),
                        lambda x: print_time(x),
                        True,
                        False,
                    ),
                ],
            )
        )

    cols = sorted(cols, key=lambda x: x.name)

    rows = [
        RowGroup(
            group.name,
            group.key,
            [
                RowHeader(inst.replace("_", "\\textunderscore "), inst)
                for inst in group.instances
            ],
            group.print_agg_row,
        )
        for group in instances_groups
    ]

    table = DataTable(cols, rows)

    inst_to_group = {
        inst: group.key for group in instances_groups for inst in group.instances
    }

    for algo_result in algo_results:
        for inst, result in algo_result.data.items():
            if inst in inst_to_group.keys():
                group = inst_to_group[inst]
                for r in result:
                    table.load_data_in_cell(r, "", group, inst, algo_result.label)

    def found_optimal_sol(cols_in_table, row_key, get_col_val):
        for algo_result in algo_results:
            if algo_result.label != "2pack":
                kernel_nodes = get_col_val(algo_result.label, "transformed")
                if kernel_nodes == 0:
                    return True

        return False

    table.finish_loading(
        OrderedDict({"transformed": (min, []), "transformed-edges": (min, [])}),
        found_optimal_sol,
    )
    table.print(out_filename)


def print_meta_table(
    instances_groups: List[InstanceGroup], out_filename, instance_meta_file
):
    cols = []

    cols.append(
        ColGroup(
            "",
            "descr",
            [
                ColHeader(
                    "$n(G)$",
                    "nodes",
                    lambda nodes: mean(nodes) if len(nodes) != 0 else 0,
                    lambda data, filename: data["nodes"],
                    round,
                    True,
                ),
                ColHeader(
                    "$m(G)$",
                    "edges",
                    lambda edges: mean(edges) if len(edges) != 0 else 0,
                    lambda data, filename: data["edges"],
                    round,
                    True,
                ),
            ],
        )
    )

    cols = sorted(cols, key=lambda x: x.name)

    rows = [
        RowGroup(
            group.name,
            group.key,
            [
                RowHeader(inst.replace("_", "\\textunderscore "), inst)
                for inst in group.instances
            ],
            group.print_agg_row,
        )
        for group in instances_groups
    ]

    table = DataTable(cols, rows)

    inst_to_group = {
        inst: group.key for group in instances_groups for inst in group.instances
    }

    with open(instance_meta_file, "r") as meta_file:
        for line in meta_file:
            meta = line.split(",")
            if meta[0] in inst_to_group.keys():
                table.load_data_in_cell(
                    {"nodes": int(meta[1]), "edges": int(meta[2])},
                    "",
                    inst_to_group[meta[0]],
                    meta[0],
                    "descr",
                )

    def found_optimal_sol(cols_in_table, row_key, get_col_val):
        return False

    table.finish_loading(OrderedDict({}), found_optimal_sol)
    table.print(out_filename)


def get_reduction_effect_distribution(filename):
    vertices_reduced = {
        "degree_one": 0,
        "degree_two": 0,
        "twin": 0,
        "clique": 0,
        "domination": 0,
        "fast_domination": 0,
    }

    with open(filename, "r") as res:
        for line in res:
            for key in vertices_reduced.keys():
                if key in line:
                    vertices_reduced[key] += int(line.split(":")[1].rstrip().strip())

    return vertices_reduced


def get_instances_from_files(files):
    instances = []
    for file in files:
        with open(file, "r") as fd:
            for line in fd:
                instances.append(line.rstrip().strip().split(".")[0])
    return instances


####################################################################
"""
SOA: Graphs: Erdos+Cactus
"""
our_algos_soa = [
    AlgoResults(
        label="red2pack elaborated",
        files=get_file_paths(
            [
                "out/erdos_graphs/on_demand_all_reductions",
                "out/cactus_graphs/on_demand_all_reductions",
            ]
        ),
        get_data=get_data_m2s_bnr,
        data={},
        agg_cols=True,
    )
    # ,
    # AlgoResults(
    #    label="2pack",
    #    files=get_file_paths(["out/erdos_graphs/no_reductions"]),
    #    get_data=get_data_m2s_bnr,
    #    data={},
    #    agg_cols=True
    # ),
    # AlgoResults(
    #    label="red2pack core",
    #    files=get_file_paths(["out/erdos_graphs/on_demand_domination_clique"]),
    #    get_data=get_data_m2s_bnr,
    #    data={},
    #    agg_cols=True
    # )
]

erdos_instances = sorted(
    get_instances_from_files(["sample_erdos_graphs.txt"]),
    key=lambda x: (
        int(x.split("aGraphErdos")[1].split("-")[0]),
        int(x.split("aGraphErdos")[1].split("-")[1]),
    ),
)

print_result_sol_time_table(
    our_algos_soa
    + [
        AlgoResults(
            label="gen2pack",
            files=get_file_paths(
                ["../../2-packing-Set/out", "../../2-packing-Set/out_cactus"]
            ),
            get_data=get_data_genetic_algo,
            data={},
            agg_cols=True,
        )
    ],
    [
        InstanceGroup(
            name="Cactus",
            key="cactus",
            instances=sorted(
                get_instances_from_files(["all_cactus_graphs.txt"]),
                key=lambda x: int(x.split("cac")[1]),
            ),
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Erdos", key="erdos", instances=erdos_instances, print_agg_row=True
        ),
    ],
    "results_soa_graphs_table.tex",
)

print_result_time_performance(
    our_algos_soa
    + [
        AlgoResults(
            label="gen2pack",
            files=get_file_paths(["../../2-packing-Set/out"]),
            get_data=get_data_genetic_algo,
            data={},
            agg_cols=True,
        )
    ],
    [
        InstanceGroup(
            name="Erdos", key="erdos", instances=erdos_instances, print_agg_row=True
        )
    ],
)

print_result_time_performance(
    our_algos_soa
    + [
        AlgoResults(
            label="gen2pack",
            files=get_file_paths(["../../2-packing-Set/out_cactus"]),
            get_data=get_data_genetic_algo,
            data={},
            agg_cols=True,
        )
    ],
    [
        InstanceGroup(
            name="Cactus",
            key="cactus",
            instances=get_instances_from_files(["all_cactus_graphs.txt"]),
            print_agg_row=True,
        )
    ],
)

"""
print_result_kernel_table(
    our_algos_erdos,
    [InstanceGroup(
        name="Erdos",
        key="erdos",
        instances=erdos_instances,
        print_agg_row=True
    )],
    "results_erdos_graphs_condensed_table.tex"
)
"""

####################################################################
"""
Comparison for our Algorithms on Social+Cluster+Planar
"""

small_social_instances = sorted(
    get_instances_from_files(["small_new_social_instances.txt"])
)
large_social_instances = sorted(
    get_instances_from_files(["large_new_social_instances.txt"])
)
planar_instances = sorted(
    get_instances_from_files(["all_planar_graphs.txt"]),
    key=lambda x: int(x.split("Outerplanar")[1]),
)

our_social_algo_results = [
    AlgoResults(
        label="red2pack elaborated",
        files=get_file_paths(
            [
                "out/social_graphs/on_demand_all_reductions",
                "out/clustering_graphs/on_demand_all_reductions",
                "out/planar_graphs/on_demand_all_reductions",
            ]
        ),
        get_data=get_data_m2s_bnr,
        data={},
        agg_cols=True,
    ),
    AlgoResults(
        label="2pack",
        files=get_file_paths(
            [
                "out/social_graphs/no_reductions",
                "out/clustering_graphs/no_reductions",
                "out/planar_graphs/no_reductions",
            ]
        ),
        get_data=get_data_m2s_bnr,
        data={},
        agg_cols=True,
    ),
    AlgoResults(
        label="red2pack core",
        files=get_file_paths(
            [
                "out/social_graphs/on_demand_domination_clique",
                "out/clustering_graphs/on_demand_domination_clique",
                "out/planar_graphs/on_demand_domination_clique",
            ]
        ),
        get_data=get_data_m2s_bnr,
        data={},
        agg_cols=True,
    ),
]


print_result_kernel_table(
    our_social_algo_results,
    [
        InstanceGroup(
            name="Small Social",
            key="small-social",
            instances=small_social_instances,
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Large Social",
            key="large-social",
            instances=large_social_instances,
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Planar", key="planar", instances=planar_instances, print_agg_row=True
        ),
    ],
    "results_first_graphs_condensed_table.tex",
)


print_result_sol_time_table(
    our_social_algo_results
    + [
        # AlgoResults(
        #    label="gen2pack",
        #    files=get_file_paths(["../../2-packing-Set/out_social", "../../2-packing-Set/out_clustering"]),
        #    get_data=get_data_genetic_algo,
        #    data={},
        #    agg_cols=False
        # )
    ],
    [
        InstanceGroup(
            name="Small Social",
            key="small-social",
            instances=small_social_instances,
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Large Social",
            key="large-social",
            instances=large_social_instances,
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Planar", key="planar", instances=planar_instances, print_agg_row=True
        ),
    ],
    "results_first_graphs_table.tex",
)

print_result_time_performance(
    our_social_algo_results
    + [
        # AlgoResults(
        #    label="gen2pack",
        #    files=get_file_paths(["../../2-packing-Set/out_social", "../../2-packing-Set/out_clustering"]),
        #    get_data=get_data_genetic_algo,
        #    data={},
        #    agg_cols=False
        # )
    ],
    [
        InstanceGroup(
            name="Social",
            key="social",
            instances=small_social_instances + large_social_instances,
            print_agg_row=True,
        )
    ],
)

print_meta_table(
    [
        InstanceGroup(
            name="Cactus",
            key="cactus",
            instances=sorted(
                get_instances_from_files(["all_cactus_graphs.txt"]),
                key=lambda x: int(x.split("cac")[1]),
            ),
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Erdos", key="erdos", instances=erdos_instances, print_agg_row=True
        ),
        InstanceGroup(
            name="Small Social",
            key="small-social",
            instances=small_social_instances,
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Large Social",
            key="large-social",
            instances=large_social_instances,
            print_agg_row=True,
        ),
        InstanceGroup(
            name="Planar",
            key="planar",
            instances=sorted(
                get_instances_from_files(["all_planar_graphs.txt"]),
                key=lambda x: int(x.split("Outerplanar")[1]),
            ),
            print_agg_row=True,
        ),
    ],
    "meta_table.tex",
    "all_instances_meta.txt",
)
