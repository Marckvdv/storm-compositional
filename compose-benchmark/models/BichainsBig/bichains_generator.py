from jinja2 import Environment, FileSystemLoader, select_autoescape
import sys
import random

env = Environment(loader=FileSystemLoader("."), trim_blocks=True)
template = env.get_template("bichains.template")

DIM = 101
LAYERS = 3
N = 5
if len(sys.argv) > 1:
    N = int(sys.argv[1])

seed = 0
if len(sys.argv) > 2:
    seed = int(sys.argv[2])

random.seed(seed)

p = {
    "N": N,
    "seed": seed,
    "DIM": DIM,
}

grid_models = [
    "windy_safe",
    "windy_unsafe",
    "calm_safe",
    "calm_unsafe",
]

grid = [
    [ random.choice(grid_models) for _ in range(N)] for _ in range(LAYERS)
]

mid = (DIM-1)//2
end = DIM-1
entrance_one = f"x={mid}&y=0"
entrance_two = f"x=0&y={mid}"

exit_one = f"x={mid}&y={end}"
exit_two = f"x={end}&y={mid}"

print(template.render(p=p, grid=grid, entrance_one=entrance_one, entrance_two=entrance_two, exit_one=exit_one, exit_two=exit_two))
