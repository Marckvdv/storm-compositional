from jinja2 import Environment, FileSystemLoader, select_autoescape
import sys
import random

env = Environment(loader=FileSystemLoader("."), trim_blocks=True)
template = env.get_template("rooms.template")

DIM = 101
N = 10
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
    [ random.choice(grid_models) for _ in range(N)] for _ in range(N)
]

forward_layers = [ [] for _ in range(N) ]
for i in range(0, N):
    for j in range(i+1):
        x, y = j, i-j
        tile = grid[x][y]

        is_top_edge = x == 0
        is_bottom_edge = y == 0
        is_edge = is_top_edge or is_bottom_edge

        is_corner = is_edge and i==N-1

        if is_corner:
            tile += "_corner"
        elif is_edge:
            tile += "_edge"

        forward_layers[i].append(tile)

backward_layers = [ [] for _ in range(N-1) ]
for i in range(0, N-1):
    for j in range(i+1):
        x, y = j, i-j
        tile = grid[x][y]

        is_top_edge = x == 0
        is_bottom_edge = y == 0
        is_edge = is_top_edge or is_bottom_edge

        is_corner = is_edge and i==N-1

        if is_edge:
            tile += "_reverse_edge"

        backward_layers[i].append(tile)

layers = forward_layers + list(reversed(backward_layers))
#print(layers)

mid = (DIM-1)//2
end = DIM-1
door_one = f"x={mid}&y=0"
door_two = f"x=0&y={mid}"
door_three = f"x={mid}&y={end}"
door_four = f"x={end}&y={mid}"

layers = layers[1:-1]

print(template.render(p=p, layers=layers, door_one=door_one, door_two=door_two, door_three=door_three, door_four=door_four))
