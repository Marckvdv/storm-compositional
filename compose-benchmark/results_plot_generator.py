import json
import sys
import os
from pathlib import Path
import re
from jinja2 import Environment, FileSystemLoader, select_autoescape

import tables_generator

OOR_VALUE = 2700
IMP_VALUE = 9000
ERROR_TOLERANCE = 8e-3

PLOT_MARKS = [
    'asterisk',
    'triangle',
    'square',
    'diamond',
    '-',
    '|',
    'Mercedes star',
    'Mercedes star flipped',
    'heart',
    'pentagon',
    'oplus',
    'otimes',
    'x',
#        'halfdiamond',
    'halfcircle',
    '10-pointed star',
    'star',
    '+',
    '*',
]

def generate_plot(benchmark_data):
    model_columns = {a: [(f'Total time {a}', 'totalTime')] for a in benchmark_data.approaches}
    data_columns = ['totalTime']
    csv_columns = [ x[0] for k,v in model_columns.items() for x in v ]
    csv_columns = ['Model'] + csv_columns

    header_line = ','.join(csv_columns)
    model_order = []

    current_name = None
    current_model = None
    current_file = None
    file_count = 0
    for name in benchmark_data.model_names:
        model = benchmark_data.group_lookup[name]
        if model != current_model:
            if current_file:
                current_file.close()

            current_file = open(f'results_plot_{file_count}.csv', 'w')
            model_order.append(model)
            file_count += 1
            current_model = model
            current_file.write(f"{header_line}\n")

        current_file.write(f"{name}")
        for approach in benchmark_data.approaches:
            if not (name, approach) in benchmark_data.contents:
                model_line = ','.join(["missing" for _ in data_columns])
            else:
                content = benchmark_data.contents[(name, approach)]
                if "error" in content:
                    if content["error"] in ["MO", "error"]:
                        v = str(OOR_VALUE)
                    elif content["error"] in ["TO"]:
                        v = str(OOR_VALUE)
                    else:
                        v = str(OOR_VALUE)
                    model_line = ','.join([v for _ in data_columns])
                else:
                    if content["gap"] > ERROR_TOLERANCE:
                        v = str(IMP_VALUE)
                        model_line = ','.join([v for _ in data_columns])
                    else:
                        model_line = ','.join([str(content[c]) for c in data_columns])

            current_file.write(f",{model_line}")
        current_file.write("\n")

    if current_file:
        current_file.close()

    generate_tex_files(model_order)
    generate_plot_marks_csv(model_order)

def generate_tex_files(model_order):
    env = Environment(loader=FileSystemLoader(".."), trim_blocks=True)
    plots_template = env.get_template("plots.tex.template")
    macros_template = env.get_template("macros.tex.template")


    plot_count = len(model_order)
    plots = [ {'i': str(i), 'name': model_order[i], 'mark': PLOT_MARKS[i]} for i in range(plot_count) ]

    with open('plots.tex', 'w') as f:
        f.write(plots_template.render(plots=plots))

    with open('macros.tex', 'w') as f:
        f.write(macros_template.render(plots=plots))

def generate_plot_marks_csv(model_order):
    with open('plot_marks.csv', 'w') as f:
        header_line = f'stringdiagram,model,mark\n'
        f.write(header_line)

        for m, plot_mark in zip(model_order, PLOT_MARKS):
            sd, model = tables_generator.MODEL_NAME_LOOKUP[m]
            f.write(f'\\texttt{{{sd}}},\\texttt{{{model}}},\\showpgfmark{{{plot_mark}}}\n')
