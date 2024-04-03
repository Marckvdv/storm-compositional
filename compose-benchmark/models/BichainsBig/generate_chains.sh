#!/bin/bash
python3 bichains_generator.py 2 > BichainsBig2.json
python3 bichains_generator.py 10 > BichainsBig10.json
python3 bichains_generator.py 20 > BichainsBig20.json
python3 bichains_generator.py 50 > BichainsBig50.json
python3 bichains_generator.py 100 > BichainsBig100.json
python3 bichains_generator.py 200 > BichainsBig200.json
python3 bichains_generator.py 500 > BichainsBig500.json
python3 bichains_generator.py 1000 > BichainsBig1000.json
python3 bichains_generator.py 1500 > BichainsBig1500.json
python3 bichains_generator.py 2000 > BichainsBig2000.json
