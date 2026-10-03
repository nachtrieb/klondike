# Check the C++ port (strategy_0) against the saved R runs, state by state.
# Needs: ../results/trace.csv and ../results/results.csv (made by run_all.sh)
#        ../ref/R_v30/gs_detail_*.rda and gs_summary.rda (R.Nachtrieb's runs, 2025)
# Needs the pyreadr package to read .rda files:  pip install pyreadr
import glob, pyreadr, pandas as pd
R = pd.concat([pyreadr.read_r(f)['gs_detail'] for f in glob.glob('../ref/R_v30/gs_detail_*.rda')])
R['i'] = R['i'].astype(int); R['seed'] = R['seed'].astype(int)
R = R.drop_duplicates(['seed', 'i'])          # gs_detail_01_20 overlaps gs_detail_0_89
S = pyreadr.read_r('../ref/R_v30/gs_summary.rda')['all_gs_summary']; S['seed'] = S.seed.astype(int)
C = pd.read_csv('../results/trace.csv'); C = C[C.strategy == 'strategy_0']
res = pd.read_csv('../results/results.csv'); res = res[res.strategy == 'strategy_0']

m = res.merge(S, on='seed', suffixes=('_cpp', '_R'))
print('final score matches R:', (m.score_cpp == m.score_R).sum(), 'of', len(m))
bad = 0
for sd in sorted(m.seed):
    rc = list(R[R.seed == sd].sort_values('i').ch); cc = list(C[C.seed == sd].sort_values('i').ch)
    ok = rc == cc
    bad += not ok
    print(f'seed {sd:3d}: {len(cc):4d} states  {"identical" if ok else "DIFFERENT"}')
print('games with any state different from R:', bad, 'of', len(m))
