import csv
import json
import statistics
import sys
from enum import Enum
from typing import Callable, List, OrderedDict, Tuple
import os


class ColHeader:
    def __init__(self, name: str, key: str, aggregator: Callable[[list], any], query_value: Callable[[any], any],
                 print_value: Callable[[any], any], aggregate_col=True, num_print=True):
        """
        :param name:
        :param key:
        :param aggregator: list of values -> aggregated value
        :param query_value: source -> value in source
        """
        self.name = name
        self.key = key
        self.aggregator = aggregator
        self.pos = 0
        self.query_value = query_value
        self.print_value = print_value
        self.aggregate_col = aggregate_col
        self.num_print = num_print


class ColGroup:
    def __init__(self, name: str, key: str, cols: List[ColHeader]):
        self.name = name
        self.key = key
        self.col_keys = [col.key for col in cols]
        self.cols = dict(zip(self.col_keys, cols))

    def __len__(self):
        return len(self.cols)


class RowHeader:
    def __init__(self, name: str, key: str):
        self.name = name
        self.key = key
        self.mark = False


class RowGroup:
    def __init__(self, name: str, key: str, rows: List[RowHeader], print_agg_row=True):
        self.name = name
        self.key = key
        self.row_keys = [row.key for row in rows]
        self.rows = dict(zip(self.row_keys, rows))
        self.print_agg_row = print_agg_row

    def __len__(self):
        return len(self.rows)


class Cell:
    def __init__(self, values: list, value: any, is_best: bool):
        self.values = values
        self.value = value
        self.is_best = is_best


class DataTable:
    class BestRowValue(Enum):
        MIN = 1,
        MAX = 2

    def __init__(self, cols: List[ColGroup], rows: List[RowGroup]):
        self.col_keys = [(col_group.key, col_key) for col_group in cols for col_key in col_group.col_keys]
        self.col_group_keys = [col_group.key for col_group in cols]
        # self.col_keys_set = {col_key for col_group in cols for col_key in col_group.col_keys}
        self.cols = dict(zip(self.col_group_keys, cols))
        self.n_cols = len(self.col_keys)
        self.row_keys = [(row_group.key, row_key) for row_group in rows for row_key in row_group.row_keys]
        self.row_group_keys = [row_group.key for row_group in rows]
        self.rows = dict(zip(self.row_group_keys, rows))
        self.n_rows = len(self.row_keys)
        # shape: [[len(rows)] * n_cols]
        self.cells = [[Cell([], 0.0, False) for _ in range(self.n_rows)] for _ in range(self.n_cols)]

        # aggregation rows (one per row group)
        self.agg_rows = [[Cell([], 0.0, False) for _ in range(self.n_cols)] for _ in range(len(self.rows))]

    def load_from_log(self, storage: str, get_col_group_key: Callable[[any, str], str],
                      get_row_key: Callable[[any, str], str], get_row_group_key: Callable[[any, str], str]):
        """
        load results from experiments from storage
        :param row_group_key:
        :param get_row_key:
        :param get_col_group_key:
        :param storage: dir to json files
        :return: none
        """
        for source_filename in os.listdir(storage):
            if source_filename.split(".")[-1] == "log":
                with open(os.path.join(storage, source_filename), "r") as source_file:
                    lines = source_file.readlines()

                    row_key = get_row_key(lines, source_filename)
                    row_group_key = get_row_group_key(lines, source_filename)
                    col_group_key = get_col_group_key(lines, source_filename)
                    print(row_key)
                    print(col_group_key)
                    # print(self.col_keys)
                    # print(self.cols.keys())

                    try:
                        self.load_data_in_cell(lines, source_filename, row_group_key, row_key, col_group_key)
                    except (ValueError, KeyError):
                        print("Warning: Skipping source {} because column or header does not exist ({},{})".format(
                            source_filename, row_key, col_group_key))

    def load_row_group(self, storage: str, row_group_key: str, get_col_group_key: Callable[[any, str], str],
                       get_row_key: Callable[[any, str], str]):
        """
        load results from experiments from storage
        :param row_group_key:
        :param get_row_key:
        :param get_col_group_key:
        :param storage: dir to json files
        :return: none
        """
        for source_filename in os.listdir(storage):
            if source_filename.split(".")[-1] == "json":
                with open(os.path.join(storage, source_filename), "r") as source_file:
                    source_j = json.load(source_file)

                    row_key = get_row_key(source_j, source_filename)
                    col_group_key = get_col_group_key(source_j, source_filename)
                    print(row_key)
                    print(col_group_key)
                    # print(self.col_keys)
                    # print(self.cols.keys())

                    try:
                        self.load_data_in_cell(source_j, source_filename, row_group_key, row_key, col_group_key)
                    except (ValueError, KeyError):
                        print("Warning: Skipping source {} because column or header does not exist ({},{})".format(
                            source_filename, row_key, col_group_key))

    def load(self, storage: str, get_col_group_key: Callable[[any, str], str],
             get_row_key: Callable[[any, str], str], get_row_group_key: Callable[[any, str], str]):
        for source_filename in os.listdir(storage):
            if source_filename.split(".")[-1] == "json":
                with open(os.path.join(storage, source_filename), "r") as source_file:
                    source_j = json.load(source_file)

                    row_key = get_row_key(source_j, source_filename)
                    row_group_key = get_row_group_key(source_j, source_filename)
                    col_group_key = get_col_group_key(source_j, source_filename)
                    # print(row_key)
                    # print(col_group_key)
                    # print(self.col_keys)
                    # print(self.cols.keys())

                    try:
                        self.load_data_in_cell(source_j, source_filename, row_group_key, row_key, col_group_key)
                    except (ValueError, KeyError):
                        print("Warning: Skipping source {} because column or header does not exist ({},{})".format(
                            source_filename, row_key, col_group_key))

    def load_data_in_cell(self, data, data_filename, row_group_key, row_key, col_group_key):
        row_idx = self.get_row_idx(row_group_key, row_key)
        for col in self.cols[col_group_key].cols.values():
            new_value = col.query_value(data, data_filename)
            # print(self.rows[row_group_key].name)
            # print(new_value)
            if new_value != None:
                self.cells[self.get_col_idx(col_group_key, col.key)][row_idx].values.append(
                    new_value)

    def finish_loading(self, match_best: OrderedDict[any, BestRowValue], is_row_marked=lambda cols, get_col_val: False):
        self.agg_values_in_cells()
        self.aggregate_over_cols()
        self.compute_best(match_best)
        self.mark_rows(is_row_marked)

    def agg_values_in_cells(self):
        cells_col_idx = 0
        for col_group in self.cols.values():
            for col in col_group.cols.values():
                for row_idx in range(self.n_rows):
                    print(self.cells[cells_col_idx][row_idx].values)
                    cell = self.cells[cells_col_idx][row_idx]
                    if len(cell.values) > 0:
                        try:
                            cell.value = col.aggregator(cell.values)
                        except ValueError:
                            print("Couldn't aggregate values {} for ({},({},{}))".format(cell.values,
                                                                                         self.row_keys[row_idx][0],
                                                                                         col_group.key, col.key))
                            quit()
                    else:
                        cell.value = None
                cells_col_idx += 1

    def compute_best(self, match_best: OrderedDict[any, Tuple[BestRowValue, List[str]]]):

        def is_best_for(cmp_col_keys, row_idx, col_group_key):
            for cmp_col_key in cmp_col_keys:
                if not self.cells[self.get_col_idx(col_group_key, cmp_col_key)][row_idx].is_best:
                    return False
            return True

        POS_COL_KEY_IN_TUPEL = 1
        POS_ROW_KEY_IN_TUPEL = 1
        for match_col_key, (best_type, depends_on) in match_best.items():
            for row_idx in range(self.n_rows):
                cells = [self.cells[col_idx][row_idx] for col_idx in range(self.n_cols) if
                         self.col_keys[col_idx][POS_COL_KEY_IN_TUPEL] == match_col_key and
                         is_best_for(depends_on, row_idx, self.col_keys[col_idx][0])]
                values = [cell.value for cell in cells if cell.value != None]

                if len(values) == 0:
                    continue

                best = self.get_best(values, best_type)

                for cell in cells:
                    cell.is_best = cell.value == best
            # best in agg_row
            for row_group_idx in range(len(self.rows)):
                cells = [self.agg_rows[row_group_idx][col_idx] for col_idx in range(self.n_cols) if
                         self.col_keys[col_idx][POS_COL_KEY_IN_TUPEL] == match_col_key and
                         self.cols[self.col_keys[col_idx][0]].cols[
                             self.col_keys[col_idx][POS_COL_KEY_IN_TUPEL]].aggregate_col]
            if len(cells) > 0:
                values = [cell.value for cell in cells]
                best = self.get_best(values, best_type)
                for cell in cells:
                    cell.is_best = cell.value == best

    def mark_rows(self, is_marked):
        POS_ROW_KEY_IN_TUPEL = 1
        for row_idx in range(self.n_rows):
            get_col_value = lambda col_group_key, col_key: self.cells[self.get_col_idx(col_group_key, col_key)][
                row_idx].value
            if is_marked(self.col_keys, get_col_value):
                self.rows[self.row_keys[row_idx][0]].rows[self.row_keys[row_idx][POS_ROW_KEY_IN_TUPEL]].mark = True

    def get_best(self, values: List[any], best_type: BestRowValue):
        best = 0
        if best_type == DataTable.BestRowValue.MAX:
            best = max(values)
        elif best_type == DataTable.BestRowValue.MIN:
            best = min(values)

        return best

    def aggregate_over_cols(self):

        def row_is_agg(row_idx, col_key):
            """check if row is considered in all aggregations for col_key"""
            for col_group_key in self.col_group_keys:
                if col_key in self.cols[col_group_key].col_keys:
                    if self.cells[self.get_col_idx(col_group_key, col_key)][row_idx].value == None:
                        return False
            return True

        for row_group_idx in range(len(self.rows)):
            row_group_key = self.row_group_keys[row_group_idx]
            if not self.rows[row_group_key].print_agg_row:
                continue
            for col_idx in range(self.n_cols):
                col_group_key, col_key = self.col_keys[col_idx]
                col = self.cols[col_group_key].cols[col_key]
                if col.aggregate_col:
                    first_row_idx = self.get_row_idx(row_group_key, self.rows[row_group_key].row_keys[0])
                    values = [self.cells[col_idx][i].value for i in
                              range(first_row_idx, first_row_idx + len(self.rows[row_group_key].rows)) if
                              row_is_agg(i, col_key)]
                    if len(values) == 0:
                        self.agg_rows[row_group_idx][col_idx].value = None
                    else:
                        try:
                            self.agg_rows[row_group_idx][col_idx].value = col.aggregator(values)
                        except:
                            print("Couldn't aggregate values of col (*,({},{}))".format(
                                col_group_key, col_key))
                            quit()

    def get_row_idx(self, row_group_key: str, row_key: str):
        return self.row_keys.index((row_group_key, row_key))

    def get_col_idx(self, col_group_key: str, col_key: str):
        return self.col_keys.index((col_group_key, col_key))

    def print(self, out_filename: str):
        with open(out_filename, "w") as file:
            # first line
            line = "\\begin{tabular}{l"
            line += "".join(["c" for i in range(self.n_cols)])
            line += "} \n \hline \n"

            # first line table
            line += "\mc{1}{l|}{graphs}"
            for col_group_key, col_key in self.col_keys:
                sep = ""
                if self.cols[col_group_key].col_keys[-1] == col_key and self.col_group_keys[-1] != col_group_key:
                    sep = "|"
                line += " & \mc{1}{c" + sep + "}{" + self.cols[col_group_key].cols[col_key].name + "}"
            line += "\\\\ \n \hline \n"
            file.write(line)

            print_agg_row = False
            for col_group_key, col_key in self.col_keys:
                if self.cols[col_group_key].cols[col_key].aggregate_col:
                    print_agg_row = True
                    break

            # print rows
            cells_row_idx = 0
            row_group_idx = 0
            for row_group_key in self.row_group_keys:

                # print second line
                line = "\n \hline \n \hline \n"
                line += "\mc{1}{l|}{" + self.rows[row_group_key].name + "}"
                for col_group_key in self.col_group_keys:
                    sep = ""
                    if self.col_group_keys[-1] != col_group_key:
                        sep = "|"
                    line += " & \mc{" + str(len(self.cols[col_group_key])) + "}{|c" + sep + "}{" + self.cols[
                        col_group_key].name + "} "
                line += "\\\\ \n"
                file.write(line)
                file.write("\hline\n")

                for row_key in self.rows[row_group_key].row_keys:

                    # line = "\n \hline \n \hline \n"
                    line = ""
                    if self.rows[row_group_key].rows[row_key].mark:
                        line += "\\rowcolor{lightgray} "
                    line += "\mc{1}{l|}{\\textit{" + self.rows[row_group_key].rows[row_key].name + "}}"

                    cells_col_idx = 0
                    for col_group_key, col_key in self.col_keys:
                        cell = self.cells[cells_col_idx][cells_row_idx]
                        print_value = self.cols[col_group_key].cols[col_key].print_value
                        num_print = self.cols[col_group_key].cols[col_key].num_print

                        sep = ""
                        if (col_group_key, col_key) != self.col_keys[-1]:
                            sep = "|"

                        if cell.value == None:
                            line += ' & \mc{1}{r' + sep + '}{-}'

                        elif cell.is_best:
                            value_str = str(print_value(cell.value)) if not num_print else '\\numprint{' + str(
                                print_value(cell.value)) + '}'
                            line += ' & \mc{1}{r' + sep + '}{\\textbf{' + value_str + '}}'
                        else:
                            value_str = str(print_value(cell.value)) if not num_print else '\\numprint{' + str(
                                print_value(cell.value)) + '}'
                            line += ' & \mc{1}{r' + sep + '}{' + value_str + '}'

                        cells_col_idx += 1

                    # always append line break except it is the last row before a new row group
                    # if row_key != self.rows[row_group_key].row_keys[-1] or row_group_key == self.row_group_keys[-1]:
                    line += ' \\\\ \n'
                    file.write(line)
                    cells_row_idx += 1

                # file.write("\hline\n")

                if print_agg_row and self.rows[row_group_key].print_agg_row:
                    line = "\n \hline \n \hline \n"
                    line += "{}"
                    for col_idx in range(self.n_cols):
                        col_group_key, col_key = self.col_keys[col_idx]
                        col = self.cols[col_group_key].cols[col_key]

                        sep = ""
                        if self.col_group_keys[-1] != col_group_key:
                            sep = "|"

                        if col.aggregate_col:
                            print_value = col.print_value
                            cell = self.agg_rows[row_group_idx][col_idx]
                            if cell.is_best:
                                line += ' & \mc{1}{|r' + sep + '}{\\textbf{\\numprint{' + str(
                                    print_value(cell.value)) + '}}}'
                            else:
                                line += ' & \mc{1}{|r' + sep + '}{\\numprint{' + str(print_value(cell.value)) + '}}'
                        else:
                            line += " & \mc{1}{|r" + sep + "}{} "
                    line += "\\\\ \n"
                    file.write(line)

                file.write("\hline\n")
                file.write("\hline\n")

                row_group_idx = + 1

            file.write("\n\end{tabular} \n")

    def print_performance(self, filename: str, col_group_keys: list, col_key: str, row_group_key, min_scale=0.0,
                              max_scale=1.0,
                              ticks=0.01):
        eval_points = round(abs(max_scale - min_scale) / ticks) + 1
        lines = [[0 for _ in range(eval_points)] for _ in range(len(col_group_keys))]
        best_values = [max([self.cells[self.get_col_idx(col_group_key, col_key)][
                                self.get_row_idx(row_group_key, row_key)].value for col_group_key in
                            col_group_keys]) for row_key in self.rows[row_group_key].row_keys]
        best_by_row_key = dict(zip(self.rows[row_group_key].row_keys, best_values))
        lines_by_col_group_key = dict(zip(col_group_keys, lines))

        performance_point = 1.0 / len(self.rows[row_group_key].row_keys)

        for col_group_key in col_group_keys:
            for row_key in self.rows[row_group_key].row_keys:
                val = self.cells[self.get_col_idx(col_group_key, col_key)][
                    self.get_row_idx(row_group_key, row_key)].value
                for tick in range(eval_points):
                    if val >= (min_scale + abs(max_scale - min_scale)*(tick / (eval_points - 1))) * best_by_row_key[row_key]:
                        lines_by_col_group_key[col_group_key][tick] += performance_point

        csv_filename = filename + '.dat'
        with open(csv_filename, 'w') as csv_file:
            csv_writer = csv.writer(csv_file, delimiter=' ',
                                    quotechar='|', quoting=csv.QUOTE_MINIMAL)

            for tick in range(eval_points):
                csv_writer.writerow(
                    [(min_scale + abs(max_scale - min_scale)*(tick / (eval_points - 1)))] + [100*lines_by_col_group_key[col_group_key][tick] for
                                                                col_group_key in col_group_keys])

        file_path = filename + ".tex"

        with open(filename + ".p", "w") as gnuplot_file:
            plot_lines = []
            plot_lines.append('set term epslatex standalone color colortext linewidth 4 size 4,3 font \",10\" fontscale 1.0\n')
            plot_lines.append('set output "' + file_path + '"\n')
            plot_lines.append('set title "Performance"\n')
            plot_lines.append('set grid xtics ytics\n')
            plot_lines.append('set xrange [' + str(min_scale) + ':' + str(max_scale) + ']\n')
            plot_lines.append('set yrange [0:100]\n')
            plot_lines.append('set xlabel "$\\\\rho$"\n')
            plot_lines.append('set ylabel "$performance(\\\\rho)~[\\\\%]$"\n')
            plot_lines.append('set key bottom right\n')
            plot_lines.append('p "' + csv_filename + '" ' + str(',\\\n\t "' + csv_filename + '" ').join(["using 1:" + str(i + 2) + " title " + '"' + self.cols[col_group_keys[i]].name.replace('\\', '\\\\') + '"' + ' with lines' for i in range(len(col_group_keys))]) + '\n')
            gnuplot_file.writelines(plot_lines)


def create_weight_col():
    return ColHeader("$w$", "weight", lambda xs: round(statistics.geometric_mean(xs)),
                     lambda j_source, source_filename: j_source["metrics"]["metrics"]["final_solution_weight"], round)


def create_report_time_col():
    return ColHeader("$t$", "time", lambda xs: statistics.geometric_mean(xs),
                     lambda j_source, source_filename: j_source["metrics"]["metrics"]["measure_points"][-2]["t"],
                     lambda x: '{:.2f}'.format(round(x, 2)), True)


SEC_TO_MICRO_SEC = 1000000


def create_avg_update_time_col():
    return ColHeader("$t_{u}$~[$\mu s$]", "u-time", lambda xs: statistics.geometric_mean(xs),
                     lambda j_source, source_filename: float(
                         j_source["metrics"]["metrics"]["dyn_mean_update_time"]) * SEC_TO_MICRO_SEC,
                     lambda x: '{:.2f}'.format(round(x, 2)), True)


def create_avg_solve_time_col():
    return ColHeader("$t_{s}$~[$\mu s$]", "s-time", lambda xs: statistics.geometric_mean(xs),
                     lambda j_source, source_filename: 0 if j_source["metrics"]["metrics"][
                                                                "mean_solve_time"] == None else float(
                         j_source["metrics"]["metrics"]["mean_solve_time"]) * SEC_TO_MICRO_SEC,
                     lambda x: '{:.2f}'.format(round(x, 2)), True)


def create_avg_explore_time_col():
    return ColHeader("$t_{e}$~[$\mu s$]", "e-time", lambda xs: statistics.geometric_mean(xs),
                     lambda j_source, source_filename: 0 if j_source["metrics"]["metrics"][
                                                                "mean_explore_time"] == None else float(
                         j_source["metrics"]["metrics"]["mean_explore_time"]) * SEC_TO_MICRO_SEC,
                     lambda x: '{:.2f}'.format(round(x, 2)), True)


def create_algo_col_group(algo: str, algo_key: str):
    return ColGroup(algo, algo_key, [create_weight_col(), create_report_time_col()])


def create_graph_set_row_groups(graph_sets):
    return [RowGroup(graph_set["name"], graph_set["key"],
                     [RowHeader(graph.replace("_", "\\textunderscore "), graph) for graph in
                      sorted(graph_set["instances"])],
                     graph_set["print_agg_row"] if "print_agg_row" in graph_set else True)
            for graph_set in graph_sets.values()]


class DatGenerator:

    def __init__(self, seeds: List[str], seed_placeholder: str):
        self.seeds = seeds
        self.seed_placeholder = seed_placeholder

    def generate_dat(self, file_path_pattern: str, out_filepath: str,
                     get_data: Callable[[any], any], get_x: Callable[[any], any], get_y: Callable[[any], any],
                     compute_agg_y: Callable[[any], any] = statistics.geometric_mean,
                     print_x: Callable[[any], str] = lambda x: str(x),
                     print_y: Callable[[any], str] = lambda y: str(y)):
        agg_run = []
        data = dict.fromkeys(self.seeds, 0)
        self.init_data(file_path_pattern, data, get_data, get_x, get_y)
        self.det_agg_run(data, agg_run, compute_agg_y)
        self.write_agg_dat(out_filepath, agg_run, print_x, print_y)

    def init_data(self, file_path_pattern: str, data, get_data, get_x, get_y):

        for seed in self.seeds:
            filename = file_path_pattern.replace(self.seed_placeholder, str(seed))

            with open(filename, 'r') as json_data_file:
                json_data = json.load(json_data_file)
                # assert data points are sorted asc by x
                data[seed] = [(get_x(data_point), get_y(data_point)) for data_point in get_data(json_data)]

    def det_agg_run(self, data, agg_run, compute_agg_y):
        read_pos = dict.fromkeys(self.seeds, 0)
        pool = dict.fromkeys(self.seeds, 0)

        for seed in self.seeds:
            # assign to each seed current data point (x,y)
            pool[seed] = data[seed][read_pos[seed]]
            read_pos[seed] += 1  # read_pos points to position of next data point (that is the one not yet in pool)

        ys = [point[1] for _, point in pool.items()]
        xs = [point[0] for _, point in pool.items()]
        agg_run.append((max(xs), compute_agg_y(ys)))

        # init
        next_seed = -1
        next_x = sys.float_info.max

        all_processed = False
        while not all_processed:
            all_processed = True

            next_seed = -1
            next_x = sys.float_info.max

            # determine next seed that minimizes next_x among current data points over all seeds
            for seed in self.seeds:
                # if there are still data points for seed
                if read_pos[seed] < len(data[seed]):
                    # if current x(current data point) < next_x
                    if data[seed][read_pos[seed]][0] < next_x:
                        next_x = data[seed][read_pos[seed]][0]
                        next_seed = seed

            # was a smaller next_x found (otherwise there is new data point which can be added to pool)
            if next_x != sys.float_info.max:
                all_processed = False

                # read next data point
                pool[next_seed] = data[next_seed][read_pos[next_seed]]

                # invariant: x(data point) >= next_x for every data point in pool

                read_pos[next_seed] += 1

                ys = [point[1] for _, point in pool.items()]
                xs = [point[0] for _, point in pool.items()]
                agg_run.append((max(xs), compute_agg_y(ys)))

    def write_agg_dat(self, out_filepath, agg_run, print_x, print_y):
        with open(out_filepath, 'w') as out_file:
            for data_point in agg_run:
                out_file.write(str(print_x(data_point[0])) + " " + str(print_y(print_y(data_point[1]))) + os.linesep)
