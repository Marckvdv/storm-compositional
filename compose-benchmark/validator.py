def validate(benchmark_data):
    valid = True

    valid &= check_for_errors(benchmark_data)
    valid &= check_consistency(benchmark_data)

    return valid

def check_consistency(benchmark_data):
    valid = True

    grouped_by_model = {}
    for (model, approach), content in benchmark_data.contents.items():
        if model not in grouped_by_model:
            grouped_by_model[model] = {}

        grouped_by_model[model][approach] = content

    inconsistent = set()
    for model, approaches in grouped_by_model.items():
        lb, ub = 0, 1

        for approach, content in approaches.items():
            if 'error' in content:
                continue

            new_lb, new_ub = float(content['lowerBound']), float(content['upperBound'])

            # Add some tolerance
            new_lb -= 1e-6
            new_ub += 1e-6

            # Check if the intersection is non-empty, i.e.
            consistent = (lb <= new_lb and ub >= new_lb) or (lb <= new_ub and ub >= new_ub) or (new_lb <= lb and new_ub >= ub) and (new_lb <= ub and new_ub >= ub)
            if not consistent:
                inconsistent.add(model)
                valid = False
            else:
                lb, ub = max(lb, new_lb), min(ub, new_ub)

    for model in sorted(inconsistent):
        print(f"WARNING: model {model} has inconsistent results")

    return valid

def check_for_errors(benchmark_data):
    valid = True

    error_models = {}

    for (model, approach), content in benchmark_data.contents.items():
        if 'error' in content:
            if content['error'] not in ['error']:
                continue

            valid = False
            if not model in error_models:
                error_models[model] = set()
            error_models[model].add(approach)

    for model, approaches in error_models.items():
        print(f"WARNING: model {model} using approach(es) {','.join(approaches)} resulted in an error")

    return valid
