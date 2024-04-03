#!/bin/python3
import json
import sys
import os
from pathlib import Path
import re

import results_plot_generator
import validator
import tables_generator

RESULTS_PATH = Path('results')
PLOTS_PATH = Path('plots')


class BenchmarkData:
    def __init__(self, contents, group_lookup, model_names, approaches, model_contents):
        self.contents = contents
        self.group_lookup = group_lookup
        self.model_names = model_names
        self.approaches = approaches
        self.model_contents = model_contents

# taken from `https://stackoverflow.com/questions/4836710/is-there-a-built-in-function-for-string-natural-sort`
def natural_sort(l):
    convert = lambda text: int(text) if text.isdigit() else text.lower()
    alphanum_key = lambda key: [convert(c) for c in re.split('([0-9]+)', key)]
    return sorted(l, key=alphanum_key)

def process_results():
    results_path = RESULTS_PATH
    plots_path = PLOTS_PATH
    if len(sys.argv) >= 2:
        results_path = sys.argv[1]
    if len(sys.argv) == 3:
        plots_path = sys.argv[2]
    if len(sys.argv) > 3:
        print('Expected at most 2 command line argument (results, plots)')
        exit(1)

    os.chdir(results_path)
    result_dirs = natural_sort(os.listdir())

    print("available results:")
    for i, file_name in enumerate(result_dirs):
        print(f"[{i}] {file_name}")

    chosen_index = int(input("input index for data processing (-1 for last): "))
    result_dir = result_dirs[chosen_index]

    print(f"generating model tables for results in directory '{result_dir}'")
    os.chdir(result_dir)

    model_dirs = sorted([ x for x in Path('.').iterdir() if x.is_dir() ])
    sub_model_dirs = sorted([ y for x in model_dirs for y in x.iterdir() if y.is_dir() ])

    #print("sub models:\n" + "\n".join(map(str, sub_model_dirs)))

    model_contents = {}
    contents = {}
    groups = {}
    for sub_model_dir in sub_model_dirs:
        result_files = Path(sub_model_dir).glob("results_*.json")
        group = sub_model_dir.parent.stem

        for result_file in result_files:
            m = re.search("results_(\w+)", str(result_file.stem))
            if not m:
                continue

            approach = m.group(1)
            with open(result_file) as f:
                content = json.load(f)
                model_name = Path(sub_model_dir).stem
                contents[(model_name, approach)] = content

                if "error" not in content:
                    model_contents[model_name] = content

                if group in groups:
                    groups[group].add(model_name)
                else:
                    groups[group] = set([model_name])

    add_virtual_approach(contents, ["Monolithic", "Pareto4"], "Baseline")
    add_virtual_approach(contents, ["ALGORITHM_A1", "ALGORITHM_A2", "ALGORITHM_B"], "Novel")

    group_lookup = { v:key for key, sublist in groups.items() for v in sublist }

    model_names = natural_sort(list(sorted(set(x[0] for x in contents))))
    approaches = natural_sort(list(sorted(set(x[1] for x in contents))))

    plots_dir = Path('..') / Path('..') / plots_path
    plots_dir.mkdir(parents=True, exist_ok=True)
    os.chdir(plots_dir)

    return BenchmarkData(contents, group_lookup, model_names, approaches, model_contents)

def add_virtual_approach(contents, merge, approach_name):
    new_contents = {}
    #min_columns = ["Total time"]

    def time_min(a, b):
        for order in ["error", "oom", "timeout"]:
            if a == order: return b
            if b == order: return a

        return a if float(a) < float(b) else b

    for (model, approach), content in contents.items():
        if approach not in merge:
            continue

        key = model, approach_name
        if key not in new_contents or "error" in new_contents[key]:
            new_contents[key] = dict(content)

        if "error" in new_contents[key]:
            old_time = new_contents[key]["error"]
        else:
            old_time = new_contents[key]["totalTime"]

        if "error" in content:
            new_time = content["error"]
        else:
            new_time = content["totalTime"]

        new_contents[key]["totalTime"] = time_min(old_time, new_time)

    contents |= new_contents

def cell_formatter(column, data):
    if column == "error":
        if data == "timeout": return "TO"
        elif data == "oom": return "OOM"
        else: return "error"
    else:
        if column in ['totalTime', 'cacheInsertionTime', 'cacheRetrievalTime', 'terminationTime']:
            return f'{data:.0f}'
        elif column in ['cacheHitRatio']:
            if data == "nan":
                return "0"
            else:
                return f'{data:.2f}'
        elif column in ['stateCount']:
            return f'{data:.1e}'
        else:
            return str(data)

# Table 1: Algorithms against baseline
def generate_table1(benchmark_data, full=False):
    data_columns = [
        ('Monolithic', ['totalTime']),
        ('Pareto4', ['totalTime', 'lowerParetoPoints']),
        ('ALGORITHM_A1', ['totalTime', 'terminationTime']),
        ('ALGORITHM_A2', ['totalTime', 'terminationTime', 'lowerParetoPoints']),
        ('ALGORITHM_B', ['totalTime', 'terminationTime', 'lowerParetoPoints']),
    ]
    model_columns = [
        ('stateCount', 'State count'),
        ('uniqueLeaves', 'Unique leaves'),
    ]

    models = [
        'BiroomsBig10',
        'BiroomsSmall100',
        'BiroomsSmall200',
        'ChainsBig100',
        'ChainsBig3500',
        'ChainsDiceLoop4-500',
        'ChainsDiceLoop5-500',
        'ChainsSmall500',
        'RoomsBig10',
        'RoomsBig500',
        'RoomsSmall500',
    ]
    if full:
        models = None
        out_file = "table1full.csv"
    else:
        out_file = "table1.csv"

    tables_generator.generate_table_csv(benchmark_data, data_columns, model_columns, out_file, models, cell_formatter)

# Table 2: Cache effectiveness
def generate_table2(benchmark_data, full=False):
    data_columns = [
        ('ALGORITHM_A1', ['totalTime', 'cacheInsertionTime', 'cacheRetrievalTime', 'cacheHitRatio', 'weightedReachabilityQueries']),
        ('ALGORITHM_A2', ['totalTime', 'cacheInsertionTime', 'cacheRetrievalTime', 'cacheHitRatio', 'weightedReachabilityQueries']),
        ('ALGORITHM_B', ['totalTime', 'cacheInsertionTime', 'cacheRetrievalTime', 'cacheHitRatio', 'weightedReachabilityQueries']),
    ]
    model_columns = [
        #('stateCount', 'State count')
    ]

    models = [
        'BiroomsBig10',
        'BiroomsSmall100',
        'BiroomsSmall200',
        'ChainsBig100',
        'ChainsBig3500',
        'ChainsDiceLoop4-500',
        'ChainsDiceLoop5-500',
        'ChainsSmall500',
        'RoomsBig10',
        'RoomsBig500',
        'RoomsSmall500',
    ]
    if full:
        models = None
        out_file = "table2full.csv"
    else:
        out_file = "table2.csv"

    tables_generator.generate_table_csv(benchmark_data, data_columns, model_columns, out_file, models, cell_formatter)

def main():
    benchmark_data = process_results()
    valid = validator.validate(benchmark_data)

    if not valid:
        print("WARNING: some results were invalid. See above.")
    else:
        print("Results seem valid")

    tables_generator.generate_useful_tables(benchmark_data, cell_formatter)

    results_plot_generator.generate_plot(benchmark_data)
    generate_table1(benchmark_data)
    #generate_table2(benchmark_data)

    #generate_table1(benchmark_data, True)
    #generate_table2(benchmark_data, True)

if __name__ == '__main__':
    main()
