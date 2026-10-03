# Paired comparison of strategies on the same deals.
# Usage: python3 compare_strategies.py [results.csv] [--list-seeds]
#   results.csv: made by klondike (columns strategy,seed,score,moves,finished,won)
#   --list-seeds: also print the seeds that one strategy wins and the other loses
#
# For each strategy: games, wins, mean finished score.
# For each pair A, B, using only the seeds both played:
#   2x2 table of wins (both / A only / B only / neither),
#   exact McNemar test (two-sided binomial test on the A-only vs B-only games),
#   mean paired difference in finished score (A - B).
# Only the games where A and B disagree tell them apart; that is the point of
# playing every strategy on the same deals.
# Search strategies (search column not "-"): counts of win / exhausted / limit, so
# wins are a lower bound and wins + limit an upper bound on winnable deals; and a
# consistency check: no deal that any strategy wins may be "exhausted" (proven unwinnable).
# For a search strategy that does not win, finished = most cards on the suits seen in the search.
import sys, itertools, pandas as pd
from math import comb

args = sys.argv[1:]
list_seeds = "--list-seeds" in args
args = [a for a in args if a != "--list-seeds"]
res = pd.read_csv(args[0] if args else "results.csv")
if "search" not in res.columns:
    res["search"] = "-"

def mcnemar_exact(b, c):
    """Two-sided exact p-value for b vs c discordant pairs (binomial, p = 1/2)."""
    n = b + c
    if n == 0:
        return 1.0
    k = min(b, c)
    p = 2 * sum(comb(n, i) for i in range(k + 1)) / 2 ** n
    return min(1.0, p)

names = list(dict.fromkeys(res.strategy))   # order of first appearance
print("strategy      games  wins    win%   mean finished")
for s in names:
    r = res[res.strategy == s]
    print(f"{s:12s} {len(r):6d} {r.won.sum():5d} {100 * r.won.mean():6.1f}%  {r.finished.mean():8.2f}")

if len(names) < 2:
    print("\n(only one strategy in the file: no pairs to compare)")
for a, b in itertools.combinations(names, 2):
    A = res[res.strategy == a].set_index("seed")
    B = res[res.strategy == b].set_index("seed")
    seeds = A.index.intersection(B.index)
    A, B = A.loc[seeds], B.loc[seeds]
    both = int((A.won & B.won).sum());   a_only = int((A.won & ~B.won.astype(bool)).sum())
    b_only = int((~A.won.astype(bool) & B.won).sum()); neither = len(seeds) - both - a_only - b_only
    d = A.finished - B.finished
    print(f"\n{a} vs {b}  ({len(seeds)} common seeds)")
    print(f"  wins: both {both}, {a} only {a_only}, {b} only {b_only}, neither {neither}")
    print(f"  exact McNemar p = {mcnemar_exact(a_only, b_only):.3g}")
    print(f"  finished score {a} - {b}: mean {d.mean():+.2f}, "
          f"{a} higher in {(d > 0).sum()}, equal {(d == 0).sum()}, lower {(d < 0).sum()}")
    if list_seeds:
        print(f"  seeds won by {a} only: {sorted(seeds[(A.won == 1) & (B.won == 0)].tolist())}")
        print(f"  seeds won by {b} only: {sorted(seeds[(A.won == 0) & (B.won == 1)].tolist())}")

won_any = set(res[res.won == 1].seed)
for s in names:
    r = res[res.strategy == s]
    if (r.search == "-").all():
        continue
    n = r.search.value_counts()
    w, e, l = n.get("win", 0), n.get("exhausted", 0), n.get("limit", 0)
    print(f"\n{s} search: win {w}, exhausted (cannot be won) {e}, limit (unknown) {l}")
    print(f"  winnable deals: between {w} ({100*w/len(r):.1f}%) and {w + l} ({100*(w + l)/len(r):.1f}%)")
    bad = sorted(set(r[r.search == "exhausted"].seed) & won_any)
    print(f"  consistency: deals won by some strategy but exhausted here: {bad if bad else 'none'}")
    if l:
        print(f"  unresolved seeds: {sorted(r[r.search == 'limit'].seed.tolist())}")

