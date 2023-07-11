import os

time_limit = 36000
reduction_style = "extended"
seeds = ["0", "10", "732", "1888"] #, "2979", "42", "2342", "113", "9211", "23421"]


modes = ["no_reductions", "on_demand_domination_clique", "domination_clique", "on_demand_all_reductions", "all_reductions"]

for mode in modes:

    if mode == "no_reductions":
        optional = "--disable_deg_one --disable_deg_two --disable_twin --disable_domination --disable_fast_domination --disable_clique"
    elif mode == "on_demand_domination_clique":
        optional = "--on_demand_two_neighborhood --disable_deg_one --disable_deg_two --disable_twin --disable_fast_domination"
    elif mode == "domination_clique":
        optional = "--disable_deg_one --disable_deg_two --disable_twin --disable_fast_domination"
    elif mode == "on_demand_all_reductions":
        optional = "--on_demand_two_neighborhood"
    else:
        optional = ""

    executable = "./branch_and_reduce/deploy/m2s_branch_and_reduce"
    jobs_file = "jobs_erdos_%s.txt" % mode
    out_dir = "out/erdos_graphs/" + mode
    instances_file = "all_erdos_instances.txt"
    instances_dir = "graphs/erdos_graphs/"

    job_lines = []


    def add_job(inst_dir, inst, red_style, tl, s, optional, o_dir):
        job_lines.append("%s %s --reduction_style2=%s --time_limit=%s --seed=%s %s >> %s\n" % (
            executable, os.path.join(inst_dir, inst), red_style, tl, s, optional,
            os.path.join(o_dir, "s" + s + "_" + inst.split(".")[0] + ".txt")))


    with open(instances_file, "r") as insts_file:
        for instance in insts_file:
            for seed in seeds:
                add_job(instances_dir, instance.rstrip().strip(), reduction_style, time_limit, seed, optional, out_dir)

    with open(jobs_file, "w") as j_f:
        j_f.writelines(job_lines)
