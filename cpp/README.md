# Klondike, C++ port of solitaire_v30 (R), with pluggable strategies

Files in this folder
- `klondike.cpp`: the whole engine. Its functions match the R files: `get_moves` = get_move.R, `make_move` = make_move.R, `sort_cols` = sort_cols.R, `pick_lookahead2` = one_step.R (this is `strategy_0`). Strategies are listed in `STRATEGIES`; `./klondike --list` prints them.
- `seed_ch.csv`: the 501 deals (seeds 0–500) made by the R `deal()` (copy of `../ref/R_v30/seed_ch.csv`), so every strategy, in C++ or R, plays identical games.
- `make_plot.py`: redraws Figure 1 of the note from `results.csv` for one strategy.
- `compare.py`: checks every state of every `strategy_0` game against the saved R runs in `../ref/R_v30/`. Needs `pip install pyreadr`.
- `compare_strategies.py`: compares strategies game by game on the same deals (see below).
- `run_all.sh`: rebuilds everything in `../results/` from scratch.

Rebuild everything (macOS or Linux; strategies 0–2 take ~1.5 min, strategy_3 ~25 min, one game at a time with progress lines):

    ./run_all.sh

Or by hand:

    g++ -O2 -std=c++17 -pthread -o klondike klondike.cpp
    ./klondike --list                                          # the strategies
    ./klondike --strategy all --out results.csv                # every strategy, every deal
    ./klondike --strategy strategy_0 --seeds 0-20,222 --out r.csv
    ./klondike --strategy strategy_0 --seeds 6 --trace trace6.csv --out r6.csv   # every state saved
    python3 make_plot.py results.csv figure1.png --strategy strategy_0 --drop 222,317
    python3 compare_strategies.py results.csv --list-seeds

Options: `--strategy` takes one name, a comma list, or `all`. `--seeds` takes ranges and lists (`0-500`, `6`, `0-20,222,317`). `--deals` takes any file in the `seed_ch.csv` format, so new or hand-made deals can be played too. `--quiet` turns off the progress output (stderr: one line per game, and every 250,000 states inside a search). `--jobs N` plays N games at once. `--append` adds to an existing `--out` file. `--node-limit N` sets how many states strategy_3 may examine per game (default 2,000,000). Results are written after every game, so an interrupted run keeps what it has.

Adding a strategy: write it in `klondike.cpp`, then add one entry (name, one-line description, play function) to `STRATEGIES`. A step-by-step strategy only needs a picker function (state, legal moves, visited states → index of the move, or -1 to stop), run by `play_stepwise`. A search strategy writes its own play function. All strategies use the same `get_moves` / `make_move`, so they play by the same rules.

`results.csv` columns
- `strategy`: strategy name.
- `seed`: deal number.
- `score`: cards on the suits when the strategy stops (same as the R score).
- `moves`: number of moves played.
- `finished`: the score after also sending any remaining cards straight to the suits.
- `won`: 1 if `finished` is 52.
- `search`: `-` for step-by-step strategies. For strategy_3: `win`, `exhausted` (every reachable state tried: the deal cannot be won under these rules) or `limit` (gave up: unknown). When strategy_3 does not win, `score` and `finished` are the most cards on the suits seen anywhere in the search.
- `nodes`: states examined by a search (0 otherwise).

`compare_strategies.txt` gives, for every pair of strategies on their common seeds: the 2×2 table of wins (both, A only, B only, neither), an exact McNemar test on the games where they disagree, and the mean difference in finished score. For search strategies it also gives the bounds on winnable deals and checks that no deal won by any strategy is called `exhausted`.

`trace.csv` (every state of every strategy_0 game, about 25 MB) is not kept in the archive. `run_all.sh` recreates it.
