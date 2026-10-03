# Reproduce Figure 1 of the note (histogram of final scores + log-normal fit).
# Usage: python3 make_plot.py results.csv figure1.png [--strategy strategy_0] [--drop 222,317]
#   --strategy: which strategy's games to plot (default strategy_0, the note's strategy)
#   --drop: leave out seeds (222 and 317 are missing from the R run in the note)
import sys, numpy as np, pandas as pd, matplotlib
matplotlib.use("Agg"); import matplotlib.pyplot as plt
from math import erf, sqrt, log
args = [a for a in sys.argv[1:]]
drop = []
if "--drop" in args:
    k = args.index("--drop"); drop = [int(x) for x in args[k + 1].split(",")]; del args[k:k + 2]
strategy = "strategy_0"
if "--strategy" in args:
    k = args.index("--strategy"); strategy = args[k + 1]; del args[k:k + 2]
res = pd.read_csv(args[0] if args else "results.csv")
res = res[res.strategy == strategy]
out = args[1] if len(args) > 1 else "figure1.png"
res = res[~res.seed.isin(drop)]
n, wins = len(res), (res.finished == 52).sum()
lose = res[res.finished < 52].score
# log-normal with the same mean and sd as the losing games, as counts per bin
m, s = lose.mean(), lose.std()
sig2 = log(1 + s**2 / m**2); mu = log(m) - sig2 / 2
cdf = lambda x: 0.5 * (1 + erf((log(x) - mu) / sqrt(2 * sig2))) if x > 0 else 0
x = np.arange(0, 41); y = [len(lose) * (cdf(v + 0.5) - cdf(v - 0.5)) for v in x]
fig, ax = plt.subplots(figsize=(8, 5))
ax.hist(res.score, bins=np.arange(-0.5, 53.5, 1), color="#69b3a2", edgecolor="white")
ax.plot(x, y, color="red")
ax.set_xlabel("Final Score"); ax.set_ylabel("Number of Games")
ax.set_title(f"Total Games = {n}, Winners = {wins}, ({100*wins/n:.0f}%)   [C++, {strategy}]", loc="left")
ax.text(25, ax.get_ylim()[1] * 0.8, f"losing games: mean {m:.1f}, sd {s:.1f}")
fig.tight_layout(); fig.savefig(out, dpi=130)
print(f"{out}: {strategy} games={n} wins={wins} losing mean={m:.2f} sd={s:.2f}")
