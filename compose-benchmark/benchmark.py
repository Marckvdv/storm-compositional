#!/bin/python3

#import pandas as pd
#import matplotlib
import subprocess
from typing import List, Any, Dict, Union
import resource
import signal
import itertools
import json
from pathlib import Path
from datetime import datetime
import sys
import argparse
import os
from multiprocessing import Pool, TimeoutError

class BenchmarkOptions:
    def __init__(self, time_limit: int, memory_limit: int, paths: Dict[str, Path]) -> None:
        self.time_limit = time_limit
        self.memdp_benchmark = memory_limit
        self.paths = paths

class BenchmarkCase:
    def __init__(self, model_name: str, model_path: Path, result_path: Path, args: List[str], reach: str, approach_name: str, benchmark_options: BenchmarkOptions) -> None:
        self.model_name = model_name
        self.model_path = model_path
        self.result_path = result_path
        self.args = args
        self.reach = reach
        self.approach_name = approach_name
        self.benchmark_options = benchmark_options

        self.timeout = False
        self.error = False
        self.oom = False

        self.name = f"{model_name}_{approach_name}"

    def timeout_handler(self, signo: int, frame) -> None:
        self.timeout = True

    #def apply_options(self) -> None:
    #    signal.signal(signal.SIGXCPU, self.timeout_handler)

    def start(self):
        #self.apply_options()

        args = [str(self.benchmark_options.paths["storm"])]
        args += ["--stringdiagram", str(self.model_path)]
        args += self.args

        process = None
        try:
            print(f"Running '{' '.join(args)}'")
            process = subprocess.run(args, capture_output=True, timeout=self.benchmark_options.time_limit)

            if process.returncode == 23:
                self.oom = True
            elif process.returncode != 0:
                self.error = True
        except subprocess.TimeoutExpired as e:
            self.timeout = True
        except Exception as e:
            self.error = True

        error_str = None
        if self.timeout: error_str = "timeout"
        elif self.error: error_str = "error"
        elif self.oom: error_str = "oom"

        if error_str: print("Error: ", error_str)

        if self.oom or self.timeout or self.error:
            with open(self.result_path / Path(f'results_{self.approach_name}.json'), 'w') as f:
                f.write(f'{{"error": "{error_str}"}}\n')

        if process:
            if process.stdout:
                with open(self.result_path / Path(f'{self.approach_name}_stdout.txt'), 'w') as f:
                    f.write(process.stdout.decode('utf-8'))
            if process.stderr:
                with open(self.result_path / Path(f'{self.approach_name}_stderr.txt'), 'w') as f:
                    f.write(process.stderr.decode('utf-8'))

class Benchmark:
    def __init__(self, cases: List[BenchmarkCase], process_count: int = 8):
        self.cases = cases
        self.process_count = process_count

    def start(self) -> None:
        pool = Pool(processes=self.process_count)
        for case in self.cases:
            pool.apply_async(case.start, ())
        pool.close()
        pool.join()
