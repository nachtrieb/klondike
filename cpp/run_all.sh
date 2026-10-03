#!/bin/sh
# Rebuild everything in ../results from scratch.
# strategy_0..2 take about 1.5 minutes together. strategy_3 (search) runs one game
# at a time with progress lines; up to ~14 s a game, so expect most of an hour.
set -e
cd "$(dirname "$0")"
g++ -O2 -std=c++17 -Wall -pthread -o klondike klondike.cpp
./klondike --list
# step-by-step strategies on every deal
./klondike --strategy strategy_0,strategy_1,strategy_2 --deals seed_ch.csv --out ../results/results.csv --quiet
# search strategy, one game at a time, added to the same file (saved after every game)
./klondike --strategy strategy_3 --deals seed_ch.csv --out ../results/results.csv --append
# strategy_0 again with every state saved, to check it against the R runs
./klondike --strategy strategy_0 --deals seed_ch.csv --out ../results/strategy_0_check.csv \
           --trace ../results/trace.csv --quiet
python3 make_plot.py ../results/results.csv ../results/figure1.png --strategy strategy_0 --drop 222,317
python3 compare.py > ../results/compare_R.txt
tail -2 ../results/compare_R.txt
python3 compare_strategies.py ../results/results.csv --list-seeds > ../results/compare_strategies.txt
cat ../results/compare_strategies.txt
rm klondike ../results/strategy_0_check.csv   # build product and duplicate; rebuild any time
