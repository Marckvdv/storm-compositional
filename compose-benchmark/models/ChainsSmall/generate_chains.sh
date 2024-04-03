#!/bin/bash
python3 chains_generator.py 2 > ChainsSmall2.json
python3 chains_generator.py 10 > ChainsSmall10.json
python3 chains_generator.py 20 > ChainsSmall20.json
python3 chains_generator.py 50 > ChainsSmall50.json
python3 chains_generator.py 100 > ChainsSmall100.json
python3 chains_generator.py 200 > ChainsSmall200.json
python3 chains_generator.py 500 > ChainsSmall500.json
