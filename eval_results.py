import os.path
from collections import OrderedDict
from dataclasses import dataclass
from statistics import geometric_mean, mean
from typing import List, Callable, Dict
import matplotlib.pyplot as plt
import numpy as np

from data_table import DataTable, ColGroup, ColHeader, RowGroup, RowHeader


@dataclass
class Result:
    instance_name: str
    time: float
    timeout: bool
    size: int
    seed: int
    kernel_nodes: int  # number of nodes after 2ps reductions
    nodes: int  # number of nodes


@dataclass
class AlgoResults:
    label: str
    files: List[str]
    get_data: Callable[[str], Result]

    data: Dict[str, List[Result]]

    def load(self):
        for file in self.files:
            result = self.get_data(file)
            if result is not None:
                if result.instance_name not in self.data.keys():
                    self.data[result.instance_name] = []
                self.data[result.instance_name].append(result)


@dataclass
class InstanceGroup:
    key: str
    name: str
    instances: List[str]
    print_agg_row: bool


def get_data_m2s_bnr(file):
    name = ""
    timeout = False
    size = 0
    seed = int(os.path.basename(file)[1:].split("_")[0])
    time = 0.0
    kernel_nodes = 0
    nodes = 0

    if not os.path.exists(file):
        return None

    with open(file, "r") as result_f:
        for line in result_f:
            if "Kernel Nodes" in line:
                kernel_nodes = int(line.split(":")[1].rstrip().strip())
            elif "-Nodes" in line:
                nodes = int(line.split(":")[1].rstrip().strip())
            elif "Size" in line:
                size = int(line.split(":")[1].rstrip().strip())
            elif "Filename" in line:
                name = line.split(":")[1].rstrip().strip()
            elif "Time found" in line:
                time = float(line.split(":")[1].rstrip().strip())
            elif "Timeout" in line:
                timeout = True

    return Result(name, time, timeout, size, seed, kernel_nodes, nodes)


def get_file_paths(dir_names: List[str]):
    file_paths = []
    for dir_name in dir_names:
        file_paths = file_paths + [os.path.join(dir_name, inst_f)
                                   for inst_f in os.listdir(dir_name)]

    return file_paths


def print_all():
    m2s_bnr_all_reductions = AlgoResults(
        label="red2pack full",
        files=get_file_paths(["out/social_graphs/all_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
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
            print("%s, %s, %s, %s, %s" % (
                instance, results[0].size, geometric_mean([r.time for r in results]), results[0].nodes,
                mean([r.kernel_nodes for r in results])))


def print_result_sol_time_kernel_table(algo_results: List[AlgoResults], instances_groups: List[InstanceGroup], out_filename):
    """
    "vr": {
        "instances": ['CR-S-L-4'],
        "name": "VR",
        "key": "vr",
        "print_agg_row": True
    }

    :param algo_results:
    :param instances_groups:
    :return:
    """

    for algo_result in algo_results:
        algo_result.load()

    cols = [ColGroup(algo_result.label, algo_result.label, [
        ColHeader("$|S|$", "size", lambda sols: geometric_mean(sols) if len(sols) != 0 and 0 not in sols else 0,
                  lambda data, filename: data.size, round),
        ColHeader("$t$ [s]", "time", lambda times: geometric_mean(times) if len(times) != 0 and 0 not in times else 0,
                  lambda data, filename: data.time,
                  lambda x: '{:.5f}'.format(round(x, 5)), True),
        ColHeader("$|\mathcal{C}|$", "transformed", lambda nodes: mean(nodes) if len(nodes) != 0 else 0,
                  lambda data, filename: data.kernel_nodes, round)
    ]) for algo_result in algo_results]

    cols = sorted(cols, key=lambda x: x.name)

    rows = [RowGroup(group.name, group.key, [RowHeader(inst.replace("_", "\textunderscore"), inst) for inst in group.instances], group.print_agg_row) for group in
            instances_groups]

    table = DataTable(cols, rows)

    inst_to_group = {inst: group.key for group in instances_groups for inst in group.instances}

    for algo_result in algo_results:
        for full_instance_name, result in algo_result.data.items():
            inst = full_instance_name.split(".")[0]
            if inst in inst_to_group.keys():
                group = inst_to_group[inst]
                for r in result:
                    table.load_data_in_cell(r, "", group, inst, algo_result.label)

    time_limit = 36000

    def found_optimal_sol(cols_in_table, get_col_val):
        for algo_result in algo_results:
            size = get_col_val(algo_result.label, "size")
            time = get_col_val(algo_result.label, "time")
            #timeout = get_col_val(algo_result.label, "timeout")

            if size is not None and size > 0 and time < time_limit: # and not timeout:
                return True

        return False

    table.finish_loading(OrderedDict({"size": (DataTable.BestRowValue.MAX, []), "time": (DataTable.BestRowValue.MIN, ["size"]), "transformed" : (DataTable.BestRowValue.MIN, [])}), found_optimal_sol)

    table.print(out_filename)


def get_reduction_effect_distribution(filename):
    vertices_reduced = {"degree_one": 0, "degree_two": 0, "twin": 0, "clique": 0, "domination": 0, "fast_domination": 0}

    with open(filename, "r") as res:
        for line in res:
            for key in vertices_reduced.keys():
                if key in line:
                    vertices_reduced[key] += int(line.split(":")[1].rstrip().strip())

    return vertices_reduced


def performance_all():
    algo_results = [
        AlgoResults(
            label="red2pack full",
            files=get_file_paths(["out/erdos_graphs/all_reductions", "out/cactus_graphs/all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="baseline",
            files=get_file_paths(["out/erdos_graphs/no_reductions", "out/cactus_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        )
    ]

    for algo in algo_results:
        algo.load()

    tau = np.arange(1, 2, 0.01)
    algos = {algo.label: algo for algo in algo_results}
    p = {algo.label: [0 for _ in tau] for algo in algo_results}

    instances = os.listdir("graphs/erdos_graphs") + os.listdir("graphs/cactus_graphs")
    n_instances = float(len(instances))

    best_results = {inst: min([geometric_mean([r.time for algo in algos.values() for r in algo.data[inst]])])
                    for inst in instances}

    for inst in instances:
        for algo in algos.values():
            time = geometric_mean([r.time for r in algo.data[inst]])
            for i, t in enumerate(tau):
                if best_results[inst] * t >= time:
                    p[algo.label][i] += 1

    # norm
    for algo, perfs in p.items():
        for i, perf in enumerate(perfs):
            perfs[i] = perf / n_instances

    fig, ax = plt.subplots()
    plt.rcParams['text.usetex'] = True

    for algo_label in algos.keys():
        ax.plot(tau, p[algo_label], label=algo_label)

    ax.set(xlabel=r'$\tau$', ylabel=r'#instances [%]: $t(\mathcal{A}, \mathcal{I}) \leq \tau t_{best}(\mathcal{I})$',
           title='Performance Time Erdos-Graphs + Cactus-Graphs')
    ax.grid()
    ax.legend()
    # plt.gca().invert_xaxis()
    plt.ylim(0, 1)

    fig.savefig("plots/performance_time_erdos_cactus_graphs.png")
    plt.show()


# print_all()
# performance_all()
# print(get_reduction_effect_distribution("out/social_graphs/all_reductions/s0_coAuthorsDBLP.txt"))
# print(get_reduction_effect_distribution("out/social_graphs/on_demand_all_reductions/s10_coAuthorsDBLP.txt"))

print_result_sol_time_kernel_table([
    AlgoResults(
        label="red2pack full",
        files=get_file_paths(["out/social_graphs/all_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="baseline",
        files=get_file_paths(["out/social_graphs/no_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="red2pack o.d. full",
        files=get_file_paths(["out/social_graphs/on_demand_all_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="red2pack o.d. dom,clique",
        files=get_file_paths(["out/social_graphs/on_demand_domination_clique"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="red2pack dom,clique",
        files=get_file_paths(["out/social_graphs/domination_clique"]),
        get_data=get_data_m2s_bnr,
        data={}
    )
], [
    InstanceGroup(
        name="social",
        key="social",
        instances=["as-22july06",
            "citationCiteseer",
            "coAuthorsCiteseer",
            "coAuthorsDBLP",
            "coPapersCiteseer",
            "coPapersDBLP",
            "email-EuAll",
            "enron",
            "loc-brightkite_edges",
            "loc-gowalla_edges",
            "PGPgiantcompo",
            "web-Google",
            "wordassociation-2011"],
        print_agg_row=True
    ),
    InstanceGroup(
        name="social mem/timeout",
        key="social_memout",
        instances=["amazon-2008",
            "cnr-2000",
            "p2p-Gnutella04",
            "soc-Slashdot0902"
            ],
        print_agg_row=False
    )

], "results_social.txt")
