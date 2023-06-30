import os

time_limit = 7200
reduction_style = "extended"
seeds = ["0", "10", "732", "1888", "2979"]
optional_no_reductions = "--disable_deg_one --disable_deg_two --disable_twin --disable_domination --disable_clique"
optional = ""

executable = "./branch_and_reduce/cmake-build-release/m2s_branch_and_reduce"
jobs_file = "jobs.txt"
out_dir = "out/cactus_graphs/all_reductions"
instances_dir = "graphs/cactus_graphs"

job_lines = []


def add_job(inst_dir, inst, red_style, tl, s, optional, o_dir):
    job_lines.append("%s %s --reduction_style2=%s --time_limit=%s --seed=%s %s >> %s\n" % (
        executable, os.path.join(inst_dir, inst), red_style, tl, s, optional,
        os.path.join(o_dir, "s" + s + "_" + inst.split(".")[0] + ".txt")))


for instance in os.listdir(instances_dir):
    for seed in seeds:
        add_job(instances_dir, instance, reduction_style, time_limit, seed, optional, out_dir)

with open(jobs_file, "w") as j_f:
    j_f.writelines(job_lines)