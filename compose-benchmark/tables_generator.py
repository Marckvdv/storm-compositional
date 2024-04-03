import re

MODEL_NAME_LOOKUP = {
    'BiroomsBig': ('Birooms', 'RmB'),
    'BiroomsSmall': ('Birooms', 'RmS'),
    'ChainsBig': ('Chains', 'RmB'),
    'ChainsDice3-': ('Chains', 'Dice3'),
    'ChainsDice4-': ('Chains', 'Dice4'),
    'ChainsDice5-': ('Chains', 'Dice5'),
    'ChainsDice3': ('Chains', 'Dice3'),
    'ChainsDice4': ('Chains', 'Dice4'),
    'ChainsDice5': ('Chains', 'Dice5'),
    'ChainsDiceLoop3-': ('ChainsLoop', 'Dice3'),
    'ChainsDiceLoop4-': ('ChainsLoop', 'Dice4'),
    'ChainsDiceLoop5-': ('ChainsLoop', 'Dice5'),
    'ChainsDiceLoop3': ('ChainsLoop', 'Dice3'),
    'ChainsDiceLoop4': ('ChainsLoop', 'Dice4'),
    'ChainsDiceLoop5': ('ChainsLoop', 'Dice5'),
    'ChainsSmall': ('Chains', 'RmS'),
    'RoomsBig': ('Rooms', 'RmB'),
    'RoomsDice': ('Rooms', 'Dice2'),
    'RoomsSmall': ('Rooms', 'RmS'),
}

def generate_table_csv(benchmark_data, data_columns, model_columns, output_path, models, cell_formatter):
    if models is None:
        models = benchmark_data.model_names

    def compact_column_name(name):
        return name.lower().replace(" ", "").replace("_", "")

    def model_formatter(model):
        for k, v in MODEL_NAME_LOOKUP.items():
            if model.startswith(k):
                sd, leaf = v
                size = model[len(k):]
                break

        return f'\\texttt{{{sd}{size}}},\\texttt{{{leaf}}}'

    with open(output_path, 'w') as f:
        header_line = 'sd,leaf'
        for column, name in model_columns:
            header_line += f',{compact_column_name(name)}'

        for approach, columns in data_columns:
            for column in columns:
                header_line += f',{approach} {column}'
        f.write(compact_column_name(header_line + '\n'))

        for model in models:
            if model not in benchmark_data.model_contents:
                continue

            model_line = f"{model_formatter(model)}"
            for column, name in model_columns:
                cell_content = benchmark_data.model_contents[model][column]
                model_line += f",{cell_formatter(column, cell_content)}"
            f.write(model_line)

            for approach, columns in data_columns:
                content = benchmark_data.contents[(model, approach)]

                if 'error' in content:
                    approach_line = ',' + ','.join(cell_formatter("error", content["error"]) for _ in columns)
                else:
                    approach_line = ',' + ','.join(cell_formatter(c, content[c]) for c in columns)
                f.write(approach_line)
            f.write('\n')

def generate_useful_tables(benchmark_data, cell_formatter):
    for (model, approach), content in benchmark_data.contents.items():
        if "error" not in content:
            all_columns = content.keys()

    header_line = 'Model,' + ','.join(all_columns)
    for approach in benchmark_data.approaches:
        with open(f'{approach}_data.csv', 'w') as f:
            f.write(header_line + '\n')

            for model in benchmark_data.model_names:
                model_line = model + ','
                content = benchmark_data.contents[(model, approach)]
                if 'error' in content:
                    model_line += ','.join([cell_formatter('error', content['error']) for _ in all_columns])
                else:
                    model_line += ','.join([cell_formatter(c, content[c]) for c in all_columns])
                model_line += '\n'
                f.write(model_line)
