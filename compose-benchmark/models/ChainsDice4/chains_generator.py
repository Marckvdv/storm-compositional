from jinja2 import Environment, FileSystemLoader, select_autoescape
import sys
import random

env = Environment(loader=FileSystemLoader("."), trim_blocks=True)
template = env.get_template("ChainsDice4.template")

N = 10
if len(sys.argv) > 1:
    N = int(sys.argv[1])

p = {
    "N": N,
}

models = [
    "middle",
]

chain = [ random.choice(models) for _ in range(N) ]
print(template.render(p=p, chain=chain))
