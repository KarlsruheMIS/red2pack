import csv
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
    kernel_edges: int  # number of nodes after 2ps reductions
    nodes: int  # number of nodes
    offset: int
    kernel_size: int
    time_kamis: float
    time_transform: float
    memout: bool


@dataclass
class AlgoResults:
    label: str
    files: List[str]
    get_data: Callable[[str], List[Result]]

    data: Dict[str, List[Result]]

    def load(self):
        for file in self.files:
            results = self.get_data(file)
            for result in results:
                if result is not None:
                    if result.instance_name not in self.data.keys():
                        self.data[result.instance_name.split(".")[0]] = []
                    self.data[result.instance_name.split(".")[0]].append(result)


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
    time = None
    kernel_nodes = 0
    kernel_edges = 0
    nodes = 0
    offset = 0
    kernel_size = 0
    time_kamis = 0.0
    time_transform=0.0
    memout = False

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
                size = int(line.split(":")[1].rstrip().strip())
            elif "Filename" in line:
                name = line.split(":")[1].rstrip().strip()
            elif "Time found" in line:
                time = float(line.split(":")[1].rstrip().strip())
            elif "Transformation time" in line or "Reduction time" in line:
                time_transform += float(line.split(":")[1].rstrip().strip())
            elif "Offset" in line:
                offset = float(line.split(":")[1].rstrip().strip())
            elif "[" in line and "]" in line:
                kernel_size = int(line.split(" [")[0])
                time_kamis = float(line.split(" [")[1].split("]")[0])
            elif "Timeout" in line:
                timeout = True

    if time is None:
        memout = True

    if not memout and time_kamis == 0.0 and kernel_nodes > 0:
        time_kamis = time - time_transform

    return [Result(name, time, timeout, size, seed, kernel_nodes, kernel_edges, nodes, offset, kernel_size, time_kamis, time_transform, memout)]


def get_data_genetic_algo(file):
    results = []

    if not os.path.exists(file) or "results_" not in file:
        return results

    with open(file, "r") as result_f:
        reader = csv.DictReader(result_f, delimiter=",")
        for row in reader:
            print(file)
            print(row.keys())
            name = row["Graph"]
            timeout = False
            size = int(float(row["GA_withImp"]))
            seed = int(row["Seed"])
            time = float(row["Init_Time"]) + float(row["Solve_Time"])
            kernel_nodes = 0
            kernel_edges = 0
            nodes = 0

            results.append(Result(name, time, timeout, size, seed, kernel_nodes, kernel_edges, nodes, 0, 0, 0, 0, size==0))

    return results

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


def print_time(time, time_limit, digits=2):
    if time is None:
        return "m.o."
    if time > time_limit:
        return "t.o."
    return '\\numprint{' + ('{:.%sf}' % digits).format(round(time, digits)) + '}'

def print_size_approx(size, digits=2):
    return '\\numprint{' + ('{:.%sf}' % digits).format(round(size, digits)) + '}'


def agg_time(times):
    if None in times:
        return None
    return geometric_mean(times)

def get_size(data):
    if data.size > 0:
        return data.size
    return data.kernel_size + data.offset

def print_result_time_performance(algo_results: List[AlgoResults], instances_groups: List[InstanceGroup], out_filename):
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


    time_limit = 36000000 # ms

    for algo_result in algo_results:
        algo_result.load()

    cols = []
    for algo_result in algo_results:
        if algo_result.label != "gen2pack":
            cols.append(ColGroup(algo_result.label, algo_result.label, [
                ColHeader("$|S|$", "size", lambda sols: geometric_mean(sols) if len(sols) != 0 and 0 not in sols else 0,
                          lambda data, filename: get_size(data), round, False),
                ColHeader("$t_p$ [ms]", "time-proof", lambda times: agg_time(times),
                          lambda data, filename: data.time*1000 if data.time is not None else None, # s to ms
                          lambda x: print_time(x, time_limit), True, False),
                ColHeader("$t$ [ms]", "time", lambda times: agg_time(times),
                          lambda data, filename: (data.time_transform+data.time_kamis)*1000, # s to ms
                          lambda x: print_time(x, time_limit), True, False)
                ]))
        else:
            cols.append(ColGroup(algo_result.label, algo_result.label, [
                ColHeader("$|S|$", "size", lambda sols: geometric_mean(sols) if len(sols) != 0 and 0 not in sols else 0,
                          lambda data, filename: get_size(data), round, False),
                ColHeader("$t$ [ms]", "time", lambda times: agg_time(times),
                          lambda data, filename: data.time*1000 if data.time is not None else None, # s to ms
                          lambda x: print_time(x, time_limit), False, False)
                ]))


    cols = sorted(cols, key=lambda x: x.name)

    rows = [RowGroup(group.name, group.key, [RowHeader(inst.replace("_", "\\textunderscore "), inst) for inst in group.instances], group.print_agg_row) for group in
            instances_groups]

    table = DataTable(cols, rows)

    inst_to_group = {inst: group.key for group in instances_groups for inst in group.instances}

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
    table.print_performance("Performance Solution Quality %s" % instances_groups[0].name, "plots/performance_sol_%s" % instances_groups[0].name, labels, "size", max, lambda x,best_scaled: x>= best_scaled, instances_groups[0].key, 1.0, 0.8, y_label=y_label_sol)
    table.print_performance("Performance Time %s" % instances_groups[0].name, "plots/performance_time_%s" % instances_groups[0].name, labels, "time", min, lambda x,best_scaled: x <= best_scaled, instances_groups[0].key, 1.0, 2.0, y_label=y_label_time)
    table.print_performance("Performance Time %s (Log. Scale)" % instances_groups[0].name, "plots/performance_time_%s_log" % instances_groups[0].name, labels, "time", min, lambda x,best_scaled: x<= best_scaled, instances_groups[0].key, 1.0, 100000.0, ticks=1.0, y_label=y_label_time, x_log_scale=True)


def print_result_sol_time_table_erdos(algo_results: List[AlgoResults], instance_group: InstanceGroup, out_filename):
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


    time_limit = 36000000 # ms

    for algo_result in algo_results:
        algo_result.load()

    cols = []
    for algo_result in algo_results:
        if algo_result.label != "gen2pack":
            cols.append(ColGroup(algo_result.label, algo_result.label, [
                ColHeader("$|S|$", "size", lambda sols: geometric_mean(sols) if len(sols) != 0 and 0 not in sols else 0,
                    lambda data, filename: get_size(data), lambda x: print_size_approx(x), True, False),
                ColHeader("$t$ [ms]", "time", lambda times: agg_time(times),
                          lambda data, filename: data.time*1000 if data.time is not None else None, # s to ms
                          lambda x: print_time(x, time_limit), True, False),
                ColHeader("$t_f$ [ms]", "time-found", lambda times: agg_time(times),
                          lambda data, filename: (data.time_transform+data.time_kamis)*1000, # s to ms
                          lambda x: print_time(x, time_limit), True, False)
                ]))
        else:
            cols.append(ColGroup(algo_result.label, algo_result.label, [
                ColHeader("$|S|$", "size", lambda sols: geometric_mean(sols) if len(sols) != 0 and 0 not in sols else 0,
                    lambda data, filename: get_size(data), lambda x: print_size_approx(x), True, False),
                ColHeader("$t$ [ms]", "time", lambda times: agg_time(times),
                          lambda data, filename: data.time*1000 if data.time is not None else None, # s to ms
                          lambda x: print_time(x, time_limit), True, False)
                ]))



    cols = sorted(cols, key=lambda x: x.name)

    def get_n(x):
        return x.split("aGraphErdos")[1].split("-")[0]


    keys = sorted(list(set([int(get_n(inst)) for inst in instance_group.instances])))
    rows = [RowGroup(instance_group.name, instance_group.key, [RowHeader(str(k), str(k)) for k in keys], instance_group.print_agg_row)]

    table = DataTable(cols, rows)

    for algo_result in algo_results:
        for inst, result in algo_result.data.items():
            group = instance_group.key
            if inst in instance_group.instances:
                for r in result:
                    table.load_data_in_cell(r, "", group, get_n(inst), algo_result.label)


    def found_optimal_sol(cols_in_table, row_key, get_col_val):
        for algo_result in algo_results:
            if algo_result.label != "gen2pack":
                size = get_col_val(algo_result.label, "size")
                time = get_col_val(algo_result.label, "time")
           
                if time is not None and time <= time_limit and size > 0:
                    return True

        return False


    table.finish_loading(OrderedDict({"size": (DataTable.BestRowValue.MAX, []), "time": (DataTable.BestRowValue.MIN, ["size"]), "time-found": (DataTable.BestRowValue.MIN, ["size"])}), found_optimal_sol)
    table.print(out_filename)



def print_result_sol_time_table(algo_results: List[AlgoResults], instances_groups: List[InstanceGroup], out_filename):
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


    time_limit = 36000000 # ms

    for algo_result in algo_results:
        algo_result.load()

    cols = []
    for algo_result in algo_results:
        if algo_result.label != "gen2pack":
            cols.append(ColGroup(algo_result.label, algo_result.label, [
                ColHeader("$|S|$", "size", lambda sols: geometric_mean(sols) if len(sols) != 0 and 0 not in sols else 0,
                  lambda data, filename: get_size(data), round, True),
                ColHeader("$t$ [ms]", "time", lambda times: agg_time(times),
                          lambda data, filename: data.time*1000 if data.time is not None else None, # s to ms
                          lambda x: print_time(x, time_limit), True, False),
                ColHeader("$t_f$ [ms]", "time-found", lambda times: agg_time(times),
                          lambda data, filename: (data.time_transform+data.time_kamis)*1000, # s to ms
                          lambda x: print_time(x, time_limit), True, False)
                ]))
        else:
            cols.append(ColGroup(algo_result.label, algo_result.label, [
                ColHeader("$|S|$", "size", lambda sols: geometric_mean(sols) if len(sols) != 0 and 0 not in sols else 0,
                  lambda data, filename: get_size(data), round, True, True),
                ColHeader("$t$ [ms]", "time", lambda times: agg_time(times),
                          lambda data, filename: data.time*1000 if data.time is not None else None, # s to ms
                          lambda x: print_time(x, time_limit), True, False)
                ]))



    cols = sorted(cols, key=lambda x: x.name)

    rows = [RowGroup(group.name, group.key, [RowHeader(inst.replace("_", "\\textunderscore "), inst) for inst in group.instances], group.print_agg_row) for group in
            instances_groups]

    table = DataTable(cols, rows)

    inst_to_group = {inst: group.key for group in instances_groups for inst in group.instances}

    for algo_result in algo_results:
        for inst, result in algo_result.data.items():
            if inst in inst_to_group.keys():
                group = inst_to_group[inst]
                for r in result:
                    table.load_data_in_cell(r, "", group, inst, algo_result.label)


    def found_optimal_sol(cols_in_table, row_key, get_col_val):
        for algo_result in algo_results:
            size = get_col_val(algo_result.label, "size")
            time = get_col_val(algo_result.label, "time")
           
            if time is not None and time <= time_limit and size > 0:
                return True

        return False


    table.finish_loading(OrderedDict({"size": (DataTable.BestRowValue.MAX, []), "time": (DataTable.BestRowValue.MIN, ["size"]), "time-found": (DataTable.BestRowValue.MIN, ["size"])}), found_optimal_sol)
    table.print(out_filename)

    #labels = sorted([algo_result.label for algo_result in algo_results])
    #y_label_time = "$\\\\#\\\\{\\\\mathrm{instances}\\\\leq \\\\tau\\\\cdot\\\\mathrm{fastest}\\\\}/\\\\#\\\\mathrm{instances}~[\\\\%]$"
    #y_label_sol = "$\\\\#\\\\{\\\\mathrm{instances}\\\\geq \\\\tau\\\\cdot\\\\mathrm{best}\\\\}/\\\\#\\\\mathrm{instances}~[\\\\%]$"
    #table.print_performance("Performance Time %s" % instances_groups[0].name, "plots/performance_time_%s" % instances_groups[0].name, labels, "time", min, lambda x,best_scaled: x <= best_scaled, instances_groups[0].key, 1.0, 2.0, y_label=y_label_time)
    #table.print_performance("Performance Solution Quality %s" % instances_groups[0].name, "plots/performance_sol_%s" % instances_groups[0].name, labels, "size", max, lambda x,best_scaled: x>= best_scaled, instances_groups[0].key, 1.0, 0.0, y_label=y_label_sol)
    #table.print_performance("Performance Time %s (Log. Scale)" % instances_groups[0].name, "plots/performance_time_%s_log" % instances_groups[0].name, labels, "time", min, lambda x,best_scaled: x<= best_scaled, instances_groups[0].key, 1.0, 100000.0, ticks=1.0, y_label=y_label_time, x_log_scale=True)


def print_result_kernel_table(algo_results: List[AlgoResults], instances_groups: List[InstanceGroup], out_filename, instance_meta_file):
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


    time_limit = 36000000 # ms

    for algo_result in algo_results:
        algo_result.load()

    cols = [ColGroup(algo_result.label, algo_result.label, [
        ColHeader("$n(\\tilde{G})$", "transformed", lambda nodes: mean(nodes) if len(nodes) != 0 else 0,
                  lambda data, filename: data.kernel_nodes, round, True),
        ColHeader("$m(\\tilde{G})$", "transformed-edges", lambda edges: mean(edges) if len(edges) != 0 else 0,
                  lambda data, filename: data.kernel_edges, round, True)#,
        #ColHeader("$t_{\\texttt{t}}$~[ms]", "transform-time", lambda tt: mean(tt),
        #          lambda data, filename: data.time_transform*1000, # s to micro ms
        #          lambda x: print_time(x, time_limit), True, False),
        #ColHeader("$t_{\\texttt{KaMIS}}~[ms]$", "kamis-time", lambda kt: mean(kt),
        #          lambda data, filename: data.time_kamis*1000, # s to micro ms
        #          lambda x: print_time(x, time_limit), True, False)
    ]) for algo_result in algo_results if algo_result.label != '2pack']

    if '2pack' in [algo_result.label for algo_result in algo_results]:
        cols.append(ColGroup('2pack', '2pack', [
            ColHeader("$m(\\tilde{G})$", "transformed-edges", lambda edges: mean(edges) if len(edges) != 0 else 0,
                  lambda data, filename: data.kernel_edges, round, True)
        ]))

    cols.append(ColGroup("", "descr", [
        ColHeader("$n(G)$", "nodes", lambda nodes: mean(nodes) if len(nodes) != 0 else 0,
                  lambda data, filename: data["nodes"], round, True),
        ColHeader("$m(G)$", "edges", lambda edges: mean(edges) if len(edges) != 0 else 0,
                  lambda data, filename: data["edges"], round, True)
    ]))


    cols = sorted(cols, key=lambda x: x.name)

    rows = [RowGroup(group.name, group.key, [RowHeader(inst.replace("_", "\\textunderscore "), inst) for inst in group.instances], group.print_agg_row) for group in
            instances_groups]

    table = DataTable(cols, rows)

    inst_to_group = {inst: group.key for group in instances_groups for inst in group.instances}

    for algo_result in algo_results:
        for inst, result in algo_result.data.items():
            if inst in inst_to_group.keys():
                group = inst_to_group[inst]
                for r in result:
                    table.load_data_in_cell(r, "", group, inst, algo_result.label)


    with open(instance_meta_file, "r") as meta_file:
        for line in meta_file:
            meta = line.split(",")
            if meta[0] in inst_to_group.keys():
                table.load_data_in_cell({"nodes": int(meta[1]), "edges": int(meta[2])}, "", inst_to_group[meta[0]], meta[0], "descr")


    def found_optimal_sol(cols_in_table, row_key, get_col_val):
        for algo_result in algo_results:
            if algo_result.label != "2pack":
                kernel_nodes = get_col_val(algo_result.label, "transformed")
                if kernel_nodes == 0:
                    return True

        return False


    table.finish_loading(OrderedDict({"transformed" : (DataTable.BestRowValue.MIN, []), "transformed-edges" : (DataTable.BestRowValue.MIN, [])}), found_optimal_sol)
    table.print(out_filename)


def print_result_summary_kernel_table(algo_results: List[AlgoResults], instances_groups: List[InstanceGroup], out_filename, instance_meta_file):
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
        ColHeader("$n(\\tilde{G})$", "transformed", lambda nodes: mean(nodes) if len(nodes) != 0 else 0,
                  lambda data, filename: data["transformed"], round, True),
        ColHeader("$m(\\tilde{G})$", "transformed-edges", lambda edges: mean(edges) if len(edges) != 0 else 0,
                  lambda data, filename: data["transformed-edges"], round, True),
        ColHeader("$t_{\\texttt{t}}$~[ms]", "transform-time", lambda tt: mean(tt) if len(tt) != 0 else 0,
                  lambda data, filename: data["transform-time"]*1000, # s to micro ms
                  lambda x: '{:.2f}'.format(round(x, 2)), True)#,
        #ColHeader("$t_{\\texttt{KaMIS}}~[$\mu$s]$", "kamis-time", lambda kt: geometric_mean(kt) if len(kt) != 0 and 0 not in kt else 0,
        #          lambda data, filename: data.time_kamis*1000000, # s to micro s
        #          lambda x: '{:.2f}'.format(round(x, 2)), True)
    ]) for algo_result in algo_results]

    cols.append(ColGroup("", "descr", [
        ColHeader("$n(G)$", "nodes", lambda nodes: mean(nodes) if len(nodes) != 0 else 0,
                  lambda data, filename: data["nodes"], round, True),
        ColHeader("$m(G)$", "edges", lambda edges: mean(edges) if len(edges) != 0 else 0,
                  lambda data, filename: data["edges"], round, True)
    ]))

    cols = sorted(cols, key=lambda x: x.name)

    rows = [RowGroup("", "classes", [RowHeader(group.name, group.key) for group in instances_groups], True)]

    table = DataTable(cols, rows)

    inst_to_group = {inst: group.key for group in instances_groups for inst in group.instances}

    for algo_result in algo_results:
        for inst, result in algo_result.data.items():
            if inst in inst_to_group.keys():
                group = inst_to_group[inst]
                table.load_data_in_cell({"transformed": mean([r.kernel_nodes for r in result]), "transformed-edges": mean([r.kernel_edges for r in result]), "transform-time": geometric_mean([r.time_transform for r in result])}, "", "classes", group, algo_result.label)

    with open(instance_meta_file, "r") as meta_file:
        for line in meta_file:
            meta = line.split(",")
            if meta[0] in inst_to_group.keys():
                table.load_data_in_cell({"nodes": int(meta[1]), "edges": int(meta[2])}, "", "classes", inst_to_group[meta[0]], "descr")

    time_limit = 36000000 # ms

    def found_optimal_sol(cols_in_table, get_col_val):
        for algo_result in algo_results:
            kernel_nodes = get_col_val(algo_result.label, "transformed")
            if kernel_nodes == 0:
                return True

        return False


    table.finish_loading(OrderedDict({"transformed" : (DataTable.BestRowValue.MIN, []), "transformed-edges" : (DataTable.BestRowValue.MIN, [])}), found_optimal_sol)
    table.print(out_filename)
 


def get_reduction_effect_distribution(filename):
    vertices_reduced = {"degree_one": 0, "degree_two": 0, "twin": 0, "clique": 0, "domination": 0, "fast_domination": 0}

    with open(filename, "r") as res:
        for line in res:
            for key in vertices_reduced.keys():
                if key in line:
                    vertices_reduced[key] += int(line.split(":")[1].rstrip().strip())

    return vertices_reduced


# print_all()
# performance_all()
# print(get_reduction_effect_distribution("out/social_graphs/all_reductions/s0_coAuthorsDBLP.txt"))
# print(get_reduction_effect_distribution("out/social_graphs/on_demand_all_reductions/s10_coAuthorsDBLP.txt"))

def get_instances_from_files(files):
    instances = []
    for file in files:
        with open(file, "r") as fd:
            for line in fd:
                instances.append(line.rstrip().strip().split(".")[0])
    return instances

'''
print_result_summary_kernel_table(
    [
        AlgoResults(
            label="2pack",
            files=get_file_paths(["out/erdos_graphs/no_reductions", "out/cactus_graphs/no_reductions", "out/social_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack full",
            files=get_file_paths(["out/erdos_graphs/on_demand_all_reductions", "out/cactus_graphs/on_demand_all_reductions", "out/social_graphs/on_demand_all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack dom+clique",
            files=get_file_paths(["out/erdos_graphs/on_demand_domination_clique", "out/cactus_graphs/on_demand_domination_clique", "out/social_graphs/on_demand_domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        )#,
       # AlgoResults(
       #     label="genetic$_{200}$",
       #     files=get_file_paths(["../../2-packing-Set/out_cactus"]),
       #     get_data=get_data_genetic_algo,
       #     data={}
       # )
    ],
    [
    InstanceGroup(
        name="Erdos",
        key="erdos",
        instances=get_instances_from_files(["all_erdos_graphs.txt"]),
        print_agg_row=False
    ),
    InstanceGroup(
        name="Cactus",
        key="cactus",
        instances=sorted(get_instances_from_files(["all_cactus_graphs.txt"]), key=lambda x: int(x.split("cac")[1])),
        print_agg_row=True
    ),
    InstanceGroup(
        name="Social",
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
    )],
    "out_summary_condensed.txt",
    "all_instances_meta.txt"
)
'''
'''
print_result_kernel_table(
    [
        AlgoResults(
            label="2pack",
            files=get_file_paths(["out/cactus_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack full",
            files=get_file_paths(["out/cactus_graphs/on_demand_all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack dom+clique",
            files=get_file_paths(["out/cactus_graphs/on_demand_domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        )
    ],
    [InstanceGroup(
        name="Cactus",
        key="cactus",
        instances=sorted(get_instances_from_files(["all_cactus_graphs.txt"]), key=lambda x: int(x.split("cac")[1])),
        print_agg_row=True
    )],
    "results_cactus_condensed.txt",
    "all_instances_meta.txt"
)

print_result_time_performance(
    [
        AlgoResults(
            label="red2pack full",
            files=get_file_paths(["out/cactus_graphs/on_demand_all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="2pack",
            files=get_file_paths(["out/cactus_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack dom+clique",
            files=get_file_paths(["out/cactus_graphs/on_demand_domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="gen2pack",
            files=get_file_paths(["../../2-packing-Set/out_cactus"]),
            get_data=get_data_genetic_algo,
            data={}
        )
    ],
    [InstanceGroup(
        name="Cactus",
        key="cactus",
        instances=sorted(get_instances_from_files(["all_cactus_graphs.txt"]), key=lambda x: int(x.split("cac")[1])),
        print_agg_row=True
    )],
    "results_cactus_performance.txt"
)
'''

'''
print_result_sol_time_table_erdos(
    [
        AlgoResults(
            label="red2pack full",
            files=get_file_paths(["out/erdos_graphs/on_demand_all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="2pack",
            files=get_file_paths(["out/erdos_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack dom+clique",
            files=get_file_paths(["out/erdos_graphs/on_demand_domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="gen2pack",
            files=get_file_paths(["../../2-packing-Set/out"]),
            get_data=get_data_genetic_algo,
            data={}
        )
    ],
    InstanceGroup(
        name="Erdos",
        key="erdos",
        instances=get_instances_from_files(["all_erdos_graphs.txt"]),
        print_agg_row=True
    ),
    "results_erdos.txt"
)
'''

'''
print_result_sol_time_table(
    [
        AlgoResults(
            label="red2pack full",
            files=get_file_paths(["out/cactus_graphs/on_demand_all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="2pack",
            files=get_file_paths(["out/cactus_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack dom+clique",
            files=get_file_paths(["out/cactus_graphs/on_demand_domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="gen2pack",
            files=get_file_paths(["../../2-packing-Set/out_cactus"]),
            get_data=get_data_genetic_algo,
            data={}
        )
    ],
    [InstanceGroup(
        name="Cactus",
        key="cactus",
        instances=sorted(get_instances_from_files(["all_cactus_graphs.txt"]), key=lambda x: int(x.split("cac")[1])),
        print_agg_row=True
    )],
    "results_cactus.txt"
)
'''

'''
print_result_sol_time_table(
    [
        AlgoResults(
            label="red2pack\\_full",
            files=get_file_paths(["out/erdos_graphs/all_reductions", "out/cactus_graphs/all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="2pack",
            files=get_file_paths(["out/erdos_graphs/no_reductions", "out/cactus_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack\\_full\\_od",
            files=get_file_paths(["out/erdos_graphs/on_demand_all_reductions", "out/cactus_graphs/on_demand_all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack\\_dom\\_clique\\_od",
            files=get_file_paths(["out/erdos_graphs/on_demand_domination_clique", "out/cactus_graphs/on_demand_domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack\\_dom\\_clique",
            files=get_file_paths(["out/erdos_graphs/domination_clique", "out/cactus_graphs/domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="genetic$_{200}$",
            files=get_file_paths(["../../2-packing-Set/out", "../../2-packing-Set/out_cactus"]),
            get_data=get_data_genetic_algo,
            data={}
        )
    ],
    [InstanceGroup(
        name="Erdos",
        key="erdos",
        instances=get_instances_from_files(["all_erdos_graphs.txt"]),
        print_agg_row=False
    )],
    "out_erdos.txt"
)
'''

'''
Social Graphs
'''
social_instances=[
            "as-22july06",
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
            "wordassociation-2011",
            "amazon-2008", 
            "cnr-2000", 
            "p2p-Gnutella04", 
            "soc-Slashdot0902"
        ]

our_social_algo_results = [
    AlgoResults(
        label="red2pack full",
        files=get_file_paths(["out/social_graphs/on_demand_all_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="2pack",
        files=get_file_paths(["out/social_graphs/no_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="red2pack dom+clique",
        files=get_file_paths(["out/social_graphs/on_demand_domination_clique"]),
        get_data=get_data_m2s_bnr,
        data={}
    )]


print_result_kernel_table(our_social_algo_results, [
    InstanceGroup(
        name="Social",
        key="social",
        instances=social_instances,
        print_agg_row=True
    )],
    "results_social_condensed.txt")


print_result_sol_time_table(our_social_algo_results, [
    InstanceGroup(
        name="Social",
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
        name="Social mem/timeout",
        key="social_timeout",
        instances=["amazon-2008", "cnr-2000", "p2p-Gnutella04", "soc-Slashdot0902"],
        print_agg_row=False
    )
], "results_social.txt")



####################################################################
'''
Cluster Graphs
'''

cluster_instance = sorted(["adjnoun",
                "astro-ph",
                "caidaRouterLevel",
                "celegans_metabolic",
                "celegansneural",
                "chesapeake",
                "cond-mat-2003",
                "cond-mat-2005",
                "cond-mat",
                "dolphins",
                "email",
                "football",
                "hep-th",
                "jazz",
                "lesmis",
                "netscience",
                "PGPgiantcompo",
                "polbooks",
                "power",
                "road_central",
                "G_n_pin_pout",
                "preferentialAttachment",
                "road_usa",
                "smallworld"
                ])

our_clustering_algo_results = [
    AlgoResults(
        label="red2pack full",
        files=get_file_paths(["out/clustering_graphs/on_demand_all_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="2pack",
        files=get_file_paths(["out/clustering_graphs/no_reductions"]),
        get_data=get_data_m2s_bnr,
        data={}
    ),
    AlgoResults(
        label="red2pack dom+clique",
        files=get_file_paths(["out/clustering_graphs/on_demand_domination_clique"]),
        get_data=get_data_m2s_bnr,
        data={}
    )]


print_result_sol_time_table(our_clustering_algo_results+
        [AlgoResults(
            label="gen2pack",
            files=get_file_paths(["../../2-packing-Set/out_clustering"]),
            get_data=get_data_genetic_algo,
            data={}
        )
        ], [
    InstanceGroup(
        name="Cluster",
        key="cluster",
        instances=cluster_instances,
        print_agg_row=True
    ) 
], "results_cluster.txt")

print_result_time_performance(our_clustering_algo_results + 
        [AlgoResults(
            label="gen2pack",
            files=get_file_paths(["../../2-packing-Set/out_clustering"]),
            get_data=get_data_genetic_algo,
            data={}
        )
        ], [
    InstanceGroup(
        name="Clustering",
        key="clustering",
        instnaces=cluster_instances,
        print_agg_row=False
    ),
], "performance_clustering.txt")

print_result_kernel_table(our_clustering_algo_results, [
    InstanceGroup(
        name="Cluster",
        key="cluster",
        instances=cluster_instances,
        print_agg_row=True
    ) 
], "results_clustering_condensed.txt", "all_instances_meta.txt")

####################################################################
'''
Planar Graphs
'''
our_algos_planar =  [
        AlgoResults(
            label="red2pack full",
            files=get_file_paths(["out/planar_graphs/on_demand_all_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="2pack",
            files=get_file_paths(["out/planar_graphs/no_reductions"]),
            get_data=get_data_m2s_bnr,
            data={}
        ),
        AlgoResults(
            label="red2pack dom+clique",
            files=get_file_paths(["out/planar_graphs/on_demand_domination_clique"]),
            get_data=get_data_m2s_bnr,
            data={}
        )
    ]

print_result_kernel_table(
    our_algos_planar,
    [InstanceGroup(
        name="Planar",
        key="planar",
        instances=sorted(get_instances_from_files(["all_planar_graphs.txt"]), key=lambda x: int(x.split("Outerplanar")[1])),
        print_agg_row=True
    )],
    "results_planar_condensed.txt",
    "all_instances_meta.txt"
)

print_result_time_performance(
    our_algos_planar,
    [InstanceGroup(
        name="Planar",
        key="planar",
        instances=sorted(get_instances_from_files(["all_planar_graphs.txt"]), key=lambda x: int(x.split("Outerplanar")[1])),
        print_agg_row=True
    )],
    "results_planar.txt"
)

print_result_sol_time_table(
    our_algos_planar,
    [InstanceGroup(
        name="Planar",
        key="planar",
        instances=sorted(get_instances_from_files(["all_planar_graphs.txt"]), key=lambda x: int(x.split("Outerplanar")[1])),
        print_agg_row=True
    )],
    "results_planar.txt"
)
