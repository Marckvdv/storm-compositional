#!/bin/bash
python3 chains_generator.py 10 > ChainsLoopSmall10.json
python3 chains_generator.py 20 > ChainsLoopSmall20.json
python3 chains_generator.py 50 > ChainsLoopSmall50.json
python3 chains_generator.py 100 > ChainsLoopSmall100.json
python3 chains_generator.py 200 > ChainsLoopSmall200.json
python3 chains_generator.py 500 > ChainsLoopSmall500.json
