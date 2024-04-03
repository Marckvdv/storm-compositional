from benchmark import *

import sys

STORM_BIN_PATH = Path('storm-compose')
RESULTS_PATH = Path('results')
MODELS_PATH = Path('models')

DEFAULT_OCVI_OPTIONS = ["--approach", "cvi", "--oviEpsilon", "1e-4", "--iterationOrder", "backward", "--cviSteps", "200000", "--paretoCacheEpsilon", "1e-5"]
LOW_PRECISION_OCVI_OPTIONS = ["--approach", "cvi", "--oviEpsilon", "1e-2", "--iterationOrder", "backward", "--cviSteps", "200000", "--paretoCacheEpsilon", "1e-2"]

approaches = [
    ("ALGORITHM_A1", DEFAULT_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "exact", "--localOviEpsilon", "0"]),
    ("ALGORITHM_A2", DEFAULT_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "pareto", "--localOviEpsilon", "0"]),
    ("ALGORITHM_B", DEFAULT_OCVI_OPTIONS + ["--useBottomUp", "--cacheMethod", "pareto"]),
    ("ALGORITHM_C1", DEFAULT_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "exact", "--localOviEpsilon", "1e-5"]),
    ("ALGORITHM_C2", DEFAULT_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "pareto", "--localOviEpsilon", "1e-5"]),

    ("ALGORITHM_A1_LP", LOW_PRECISION_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "exact", "--localOviEpsilon", "0"]),
    ("ALGORITHM_A2_LP", LOW_PRECISION_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "pareto", "--localOviEpsilon", "0"]),
    ("ALGORITHM_B_LP", LOW_PRECISION_OCVI_OPTIONS + ["--useBottomUp", "--cacheMethod", "pareto"]),
    ("ALGORITHM_C1_LP", LOW_PRECISION_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "exact", "--localOviEpsilon", "1e-5"]),
    ("ALGORITHM_C2_LP", LOW_PRECISION_OCVI_OPTIONS + ["--useOvi", "--cacheMethod", "pareto", "--localOviEpsilon", "1e-5"]),

    ("Pareto4", ["--approach", "naive", "--paretoPrecision", "1e-4"]),
    ("Monolithic", ["--approach", "monolithic", "--precision", "1e-4"]),
]

models = [
    "BiroomsBig/BiroomsBig10.json",
    "BiroomsBig/BiroomsBig20.json",
    "BiroomsBig/BiroomsBig50.json",
    "BiroomsBig/BiroomsBig100.json",
    "BiroomsBig/BiroomsBig200.json",
    "BiroomsBig/BiroomsBig500.json",

    "BiroomsSmall/BiroomsSmall10.json",
    "BiroomsSmall/BiroomsSmall20.json",
    "BiroomsSmall/BiroomsSmall50.json",
    "BiroomsSmall/BiroomsSmall100.json",
    "BiroomsSmall/BiroomsSmall200.json",
    "BiroomsSmall/BiroomsSmall500.json",

    "ChainsBig/ChainsBig10.json",
    "ChainsBig/ChainsBig20.json",
    "ChainsBig/ChainsBig50.json",
    "ChainsBig/ChainsBig100.json",
    "ChainsBig/ChainsBig200.json",
    "ChainsBig/ChainsBig500.json",
    "ChainsBig/ChainsBig1000.json",
    "ChainsBig/ChainsBig1500.json",
    "ChainsBig/ChainsBig2000.json",
    "ChainsBig/ChainsBig2500.json",
    "ChainsBig/ChainsBig3000.json",
    "ChainsBig/ChainsBig3500.json",
    "ChainsBig/ChainsBig4000.json",

    "ChainsSmall/ChainsSmall10.json",
    "ChainsSmall/ChainsSmall20.json",
    "ChainsSmall/ChainsSmall50.json",
    "ChainsSmall/ChainsSmall100.json",
    "ChainsSmall/ChainsSmall200.json",
    "ChainsSmall/ChainsSmall500.json",

    "ChainsDice3/ChainsDice3-3.json",
    "ChainsDice3/ChainsDice3-10.json",
    "ChainsDice3/ChainsDice3-20.json",
    "ChainsDice3/ChainsDice3-50.json",
    "ChainsDice3/ChainsDice3-100.json",
    "ChainsDice3/ChainsDice3-200.json",
    "ChainsDice3/ChainsDice3-300.json",
    "ChainsDice3/ChainsDice3-400.json",
    "ChainsDice3/ChainsDice3-500.json",

    "ChainsDice4/ChainsDice4-3.json",
    "ChainsDice4/ChainsDice4-10.json",
    "ChainsDice4/ChainsDice4-20.json",
    "ChainsDice4/ChainsDice4-50.json",
    "ChainsDice4/ChainsDice4-100.json",
    "ChainsDice4/ChainsDice4-200.json",
    "ChainsDice4/ChainsDice4-500.json",

    "ChainsDice5/ChainsDice5-3.json",
    "ChainsDice5/ChainsDice5-10.json",
    "ChainsDice5/ChainsDice5-20.json",
    "ChainsDice5/ChainsDice5-50.json",
    "ChainsDice5/ChainsDice5-100.json",
    "ChainsDice5/ChainsDice5-200.json",
    "ChainsDice5/ChainsDice5-500.json",

    "RoomsBig/RoomsBig10.json",
    "RoomsBig/RoomsBig20.json",
    "RoomsBig/RoomsBig50.json",
    "RoomsBig/RoomsBig100.json",
    "RoomsBig/RoomsBig200.json",
    "RoomsBig/RoomsBig250.json",
    "RoomsBig/RoomsBig500.json",

    "RoomsSmall/RoomsSmall10.json",
    "RoomsSmall/RoomsSmall20.json",
    "RoomsSmall/RoomsSmall50.json",
    "RoomsSmall/RoomsSmall100.json",
    "RoomsSmall/RoomsSmall200.json",
    "RoomsSmall/RoomsSmall500.json",

    "RoomsDice/RoomsDice10.json",
    "RoomsDice/RoomsDice20.json",
    "RoomsDice/RoomsDice50.json",
    "RoomsDice/RoomsDice100.json",
    "RoomsDice/RoomsDice200.json",
    "RoomsDice/RoomsDice250.json",
    "RoomsDice/RoomsDice300.json",
    "RoomsDice/RoomsDice350.json",
    "RoomsDice/RoomsDice400.json",
    "RoomsDice/RoomsDice450.json",
    "RoomsDice/RoomsDice500.json",

#    "Birooms3/Birooms3-10.json",
#    "Birooms3/Birooms3-20.json",
#    "Birooms3/Birooms3-50.json",
#    "Birooms3/Birooms3-100.json",
#    "Birooms3/Birooms3-200.json",
#    "Birooms3/Birooms3-500.json",

#    "BichainsBig/BichainsBig10.json",
#    "BichainsBig/BichainsBig20.json",
#    "BichainsBig/BichainsBig50.json",
#    "BichainsBig/BichainsBig100.json",
#    "BichainsBig/BichainsBig200.json",
#    "BichainsBig/BichainsBig500.json",

#    "ChainsLoopBig/ChainsLoopBig10.json",
#    "ChainsLoopBig/ChainsLoopBig20.json",
#    "ChainsLoopBig/ChainsLoopBig50.json",
#    "ChainsLoopBig/ChainsLoopBig100.json",
#    "ChainsLoopBig/ChainsLoopBig200.json",
#    "ChainsLoopBig/ChainsLoopBig500.json",
#    "ChainsLoopBig/ChainsLoopBig1000.json",
#    "ChainsLoopBig/ChainsLoopBig1500.json",
#    "ChainsLoopBig/ChainsLoopBig2000.json",
#    "ChainsLoopBig/ChainsLoopBig2500.json",
#    "ChainsLoopBig/ChainsLoopBig3000.json",
#    "ChainsLoopBig/ChainsLoopBig3500.json",
#    "ChainsLoopBig/ChainsLoopBig4000.json",
#
#    "ChainsLoopSmall/ChainsLoopSmall10.json",
#    "ChainsLoopSmall/ChainsLoopSmall20.json",
#    "ChainsLoopSmall/ChainsLoopSmall50.json",
#    "ChainsLoopSmall/ChainsLoopSmall100.json",
#    "ChainsLoopSmall/ChainsLoopSmall200.json",
#    "ChainsLoopSmall/ChainsLoopSmall500.json",
#
    "ChainsDiceLoop3/ChainsDiceLoop3-3.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-10.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-20.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-50.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-100.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-200.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-300.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-400.json",
    "ChainsDiceLoop3/ChainsDiceLoop3-500.json",

    "ChainsDiceLoop4/ChainsDiceLoop4-3.json",
    "ChainsDiceLoop4/ChainsDiceLoop4-10.json",
    "ChainsDiceLoop4/ChainsDiceLoop4-20.json",
    "ChainsDiceLoop4/ChainsDiceLoop4-50.json",
    "ChainsDiceLoop4/ChainsDiceLoop4-100.json",
    "ChainsDiceLoop4/ChainsDiceLoop4-200.json",
    "ChainsDiceLoop4/ChainsDiceLoop4-500.json",

    "ChainsDiceLoop5/ChainsDiceLoop5-3.json",
    "ChainsDiceLoop5/ChainsDiceLoop5-10.json",
    "ChainsDiceLoop5/ChainsDiceLoop5-20.json",
    "ChainsDiceLoop5/ChainsDiceLoop5-50.json",
    "ChainsDiceLoop5/ChainsDiceLoop5-100.json",
    "ChainsDiceLoop5/ChainsDiceLoop5-200.json",
    "ChainsDiceLoop5/ChainsDiceLoop5-500.json",
]
print("total cases to run:", len(models)*len(approaches))

parser = argparse.ArgumentParser(description='MEMDP benchmark')
parser.add_argument('--timelimit', dest='time_limit', type=int, default=600)
parser.add_argument('--memlimit', dest='memory_limit', type=int, default=8192)
parser.add_argument('--threads', dest='threads', type=int, default=8)
parser.add_argument('--path', dest='path', type=str)
args = parser.parse_args()

paths = {
    "storm": STORM_BIN_PATH,
    "results": RESULTS_PATH,
    "models": MODELS_PATH,
}

options = BenchmarkOptions(args.time_limit, args.memory_limit, paths)

result_path = Path(RESULTS_PATH)

if args.path is not None:
    result_path = result_path / args.path
    result_path.mkdir(parents=True, exist_ok=True)
else:
    today = datetime.today()
    current_date_string = today.strftime("%Y_%m_%d__%H_%M_%S")
    result_path = result_path / current_date_string
    result_path.mkdir(parents=True, exist_ok=False)

cases = []
for (approach_name, arguments), model in itertools.product(approaches, models):
    model_dir_path = result_path / Path(model)
    data_path = model_dir_path / Path(f"results_{approach_name}.json")

    model_dir_path.mkdir(parents=True, exist_ok=True)

    extra_args = ["--benchmarkData", str(data_path)] + arguments
    cases += [
        BenchmarkCase(model, (MODELS_PATH / model).with_suffix(".json"), model_dir_path, extra_args, "target", approach_name, options),
    ]

benchmark = Benchmark(cases, args.threads)
benchmark.start()

print(f"Finished benchmark, wrote to: {result_path}")
