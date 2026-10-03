// klondike.cpp — C++ port of R.Nachtrieb's solitaire_v30 (R), 2026-10
//
// Same model as the R code / the note:
//   * 52 cards, id 1..52 = AC..KC, AD..KD, AH..KH, AS..KS
//   * 12 columns: 1-4 suits (C,D,H,S), 5-11 piles, 12 hand
//   * a state gives, for every card, its column and its position*sign
//     (sign +1 = face up, -1 = face down) — the 104-number vector
//   * the hand uses the every-third-card-up pattern (draw three)
//
// Build:  g++ -O2 -std=c++17 -pthread -o klondike klondike.cpp
// Run:    ./klondike --strategy strategy_0 --seeds 0-500 --out results.csv
//          ./klondike --list                       (show the strategies)
//   --strategy  one name, a comma list, or "all"         (default strategy_0)
//   --seeds     e.g. 0-500, or 6, or 0-20,222,317        (default: every deal)
//   --deals     deal file, seed + 104 numbers per row    (default seed_ch.csv)
//   --trace     also save every state of every game
//   --quiet     no per-game progress lines
//   --jobs N    play N games at once (default 1)
//   --append    add to --out instead of starting it again (no header)
//   --node-limit N   states strategy_3 may examine per game (default 2000000)

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <cstdint>
#include <functional>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

// ---------------------------------------------------------------- deck
const int NCARD = 52;
const int HAND = 12;                 // hand column
const int FIRST_PILE = 5, LAST_PILE = 11;

// cards are stored 0..51 internally; id = card+1 as in the R code
inline int suit(int c)  { return c / 13; }          // 0..3 = C,D,H,S
inline int face(int c)  { return c % 13 + 1; }      // 1..13 = A..K
inline int color(int c) { return (suit(c) == 1 || suit(c) == 2) ? 1 : 0; } // red = 1
inline bool is_ace(int c)  { return face(c) == 1; }
inline bool is_king(int c) { return face(c) == 13; }

std::string label(int c) {
    const char* F = "A23456789TJQK";
    const char* S = "CDHS";
    return std::string(1, F[face(c) - 1]) + S[suit(c)];
}

// ---------------------------------------------------------------- state
struct State {
    std::array<int8_t, NCARD> col;   // column 1..12
    std::array<int8_t, NCARD> pos;   // position 1..24, negative = face down

    bool up(int c)   const { return pos[c] > 0; }
    int  where(int c) const { return std::abs(pos[c]); }
    int  score() const {             // cards on the suit piles
        int n = 0;
        for (int c = 0; c < NCARD; c++) n += (col[c] < 5);
        return n;
    }
    std::string key() const {        // for "have we been here before?"
        return std::string((const char*)col.data(), NCARD) +
               std::string((const char*)pos.data(), NCARD);
    }
    std::string csv() const {        // same text format as the R "ch"
        std::string s;
        for (int c = 0; c < NCARD; c++) s += std::to_string(col[c]) + ",";
        for (int c = 0; c < NCARD; c++) s += std::to_string(pos[c]) + (c < NCARD - 1 ? "," : "");
        return s;
    }
};

// Count and top card of each pile (index 0..6 = column 5..11). top = -1 if empty.
struct Piles { int cnt[7]; int top[7]; };

Piles pile_info(const State& s) {
    Piles p;
    for (int j = 0; j < 7; j++) { p.cnt[j] = 0; p.top[j] = -1; }
    for (int c = 0; c < NCARD; c++)
        if (s.col[c] >= FIRST_PILE && s.col[c] <= LAST_PILE) p.cnt[s.col[c] - FIRST_PILE]++;
    for (int c = 0; c < NCARD; c++)
        if (s.col[c] >= FIRST_PILE && s.col[c] <= LAST_PILE &&
            s.where(c) == p.cnt[s.col[c] - FIRST_PILE]) p.top[s.col[c] - FIRST_PILE] = c;
    return p;
}

// The reset pattern for a hand of n cards: down, down, up, ... and the last card up.
std::vector<int> hand_pattern(int n) {
    std::vector<int> pat(n);
    for (int i = 0; i < n; i++) pat[i] = (i % 3 == 2) ? 1 : -1;
    if (n > 0) pat[n - 1] = 1;
    return pat;
}

// sort_cols.R: order the piles by (count*53 + id of top card) so equal games compare equal
void sort_cols(State& s) {
    Piles p = pile_info(s);
    int val[7], idx[7];
    for (int j = 0; j < 7; j++) { val[j] = p.cnt[j] * 53 + (p.top[j] + 1); idx[j] = j; }
    std::stable_sort(idx, idx + 7, [&](int a, int b) { return val[a] < val[b]; });
    int newcol[7];                        // old pile idx[i] becomes pile i
    for (int i = 0; i < 7; i++) newcol[idx[i]] = i;
    for (int c = 0; c < NCARD; c++)
        if (s.col[c] >= FIRST_PILE && s.col[c] <= LAST_PILE)
            s.col[c] = FIRST_PILE + newcol[s.col[c] - FIRST_PILE];
}

// ---------------------------------------------------------------- moves
enum Kind { RESET = 0, ACE_TO_SUIT = 1, CARD_TO_SUIT = 2, KING_TO_EMPTY = 3, TO_PILE = 4 };
const char* KIND_NAME[] = {"reset hand", "ace to suit", "non-ace to suit", "king to empty", "to pile"};

struct Move { int kind, card, to_col, to_pos; };

// get_move.R — same moves, same order
std::vector<Move> get_moves(const State& s) {
    std::vector<Move> moves;
    Piles p = pile_info(s);

    std::vector<int> hand, hand_up, pile_up;    // all in card-id order
    for (int c = 0; c < NCARD; c++) {
        if (s.col[c] == HAND) { hand.push_back(c); if (s.up(c)) hand_up.push_back(c); }
        if (s.col[c] >= FIRST_PILE && s.col[c] <= LAST_PILE && s.up(c)) pile_up.push_back(c);
    }

    // (5) reset the hand, if it is not already in the reset pattern
    {
        std::vector<int> pat = hand_pattern(hand.size());
        bool already = true;
        for (int c : hand) if ((s.up(c) ? 1 : -1) != pat[s.where(c) - 1]) already = false;
        if (!already && !hand_up.empty()) {
            int best = hand_up[0];
            for (int c : hand_up) if (s.where(c) > s.where(best)) best = c;
            moves.push_back({RESET, best, HAND, s.where(best)});
        }
    }

    // (1),(2) pile tops, then face-up hand cards, to the suits
    std::vector<int> cand;
    for (int j = 0; j < 7; j++) if (p.top[j] >= 0) cand.push_back(p.top[j]);
    cand.insert(cand.end(), hand_up.begin(), hand_up.end());
    for (int c : cand) {
        if (is_ace(c))           moves.push_back({ACE_TO_SUIT, c, suit(c) + 1, face(c)});
        else if (s.col[c - 1] < 5) moves.push_back({CARD_TO_SUIT, c, suit(c) + 1, face(c)});
    }

    // (3) a face-up king (not already at the bottom of a pile) to an empty pile
    for (int k = 12; k < NCARD; k += 13) {
        bool movable = s.up(k) && ((s.col[k] >= FIRST_PILE && s.col[k] <= LAST_PILE && s.where(k) > 1)
                                   || s.col[k] == HAND);
        if (!movable) continue;
        for (int j = 0; j < 7; j++)
            if (p.cnt[j] == 0) moves.push_back({KING_TO_EMPTY, k, FIRST_PILE + j, 1});
    }

    // (4) a face-up card (with whatever sits on it) onto a pile top
    std::vector<int> all_up = pile_up;
    all_up.insert(all_up.end(), hand_up.begin(), hand_up.end());
    for (int j = 0; j < 7; j++) {
        int t = p.top[j];
        if (t < 0) continue;
        for (int c : all_up)
            if (face(c) == face(t) - 1 && color(c) != color(t) && s.col[c] != s.col[t])
                moves.push_back({TO_PILE, c, s.col[t], s.where(t) + 1});
    }
    return moves;
}

// make_move.R
State make_move(const State& s, const Move& m) {
    State r = s;
    int from_col = s.col[m.card], from_pos = s.where(m.card);

    // which cards travel: a pile card takes everything above it to another pile
    std::vector<int> movers = {m.card};
    if (from_col < HAND && m.to_col > 4) {
        movers.clear();
        for (int c = 0; c < NCARD; c++)
            if (s.col[c] == from_col && s.where(c) >= from_pos) movers.push_back(c);
    }
    for (int c : movers) {
        r.col[c] = m.to_col;
        r.pos[c] = m.to_pos + s.where(c) - from_pos;   // face up
    }

    if (from_col < HAND && from_pos > 1) {           // pile: turn up the card underneath
        for (int c = 0; c < NCARD; c++)
            if (s.col[c] == from_col && s.where(c) == from_pos - 1) r.pos[c] = std::abs(r.pos[c]);
    } else if (from_col == HAND) {                   // hand: cards passed go down, one below goes up
        for (int c = 0; c < NCARD; c++) {
            if (s.col[c] != HAND) continue;
            int w = s.where(c);
            if (w < from_pos)      r.pos[c] = -std::abs(r.pos[c]);
            if (w == from_pos - 1) r.pos[c] =  std::abs(r.pos[c]);
            if (w > from_pos) {                      // close the gap, keep the sign
                int sg = r.pos[c] > 0 ? 1 : -1;
                r.pos[c] = sg * (std::abs(r.pos[c]) - 1);
            }
        }
    }

    if (from_col == m.to_col && from_pos == m.to_pos) {   // reset the hand
        std::vector<int> hand;
        for (int c = 0; c < NCARD; c++) if (r.col[c] == HAND) hand.push_back(c);
        std::stable_sort(hand.begin(), hand.end(),
                         [&](int a, int b) { return r.where(a) < r.where(b); });
        std::vector<int> pat = hand_pattern(hand.size());
        for (size_t i = 0; i < hand.size(); i++) r.pos[hand[i]] = pat[i] * r.where(hand[i]);
    }
    return r;
}

State apply(const State& s, const Move& m) { State r = make_move(s, m); sort_cols(r); return r; }

// ---------------------------------------------------------------- strategies
// A strategy plays one whole game from a deal and returns a Result.
// Two kinds fit:
//   * step by step: a "picker" chooses the next move from the legal ones, and
//     play_stepwise() runs it until it returns -1 (strategy_0 works this way);
//   * whole game: a search/backtracking strategy writes its own play function.
// To add a strategy: write it, then add one entry to STRATEGIES (bottom of this section).
// Every strategy plays the same deals with the same rules (get_moves / make_move),
// so results can be compared game by game.

using Visited = std::unordered_set<std::string>;
using Picker  = std::function<int(const State&, const std::vector<Move>&, const Visited&)>;

// search: "-" for step-by-step strategies; for strategy_3 "win", "exhausted"
// (every reachable state tried, no win: the deal cannot be won) or "limit" (gave up).
struct Result { std::string strategy; int seed, score, moves, finished; std::string search = "-"; long nodes = 0; };

// After a strategy stops, keep sending cards to the suits (and resetting the hand)
// while that is possible. Tells us whether the stopping point was really a dead end.
int finish_score(State s) {
    for (int guard = 0; guard < 1000; guard++) {
        bool moved = false;
        for (const Move& m : get_moves(s))
            if (m.kind == ACE_TO_SUIT || m.kind == CARD_TO_SUIT) { s = apply(s, m); moved = true; break; }
        if (!moved) {
            std::vector<Move> ms = get_moves(s);
            if (!ms.empty() && ms[0].kind == RESET) s = apply(s, ms[0]); else break;
            bool any = false;
            for (const Move& m : get_moves(s)) any |= (m.kind == ACE_TO_SUIT || m.kind == CARD_TO_SUIT);
            if (!any) break;
        }
    }
    return s.score();
}

// Run a step-by-step strategy from the deal until its picker returns -1.
Result play_stepwise(const std::string& name, const Picker& pick, int seed, State s, std::ostream* trace) {
    sort_cols(s);
    Visited seen;
    int step = 1;
    while (true) {
        seen.insert(s.key());
        if (trace) *trace << name << "," << seed << "," << step << ",\"" << s.csv() << "\"\n";
        std::vector<Move> moves = get_moves(s);
        int k = pick(s, moves, seen);
        if (k < 0) break;
        s = apply(s, moves[k]);
        step++;
    }
    return {name, seed, s.score(), step - 1, finish_score(s)};
}

// strategy_0 — one_step.R: for every move, count the two-move paths that follow it
// (sum over replies of the number of moves after the reply, i.e. mean(N2*N3)).
// Skip moves that lead to a state already visited. Pick the highest; ties -> first.
// Returns -1 when every score is 0, as in the R code.
int pick_lookahead2(const State& s, const std::vector<Move>& moves, const Visited& seen) {
    long best_score = 0; int best = -1;
    for (size_t i = 0; i < moves.size(); i++) {
        State s1 = apply(s, moves[i]);
        if (seen.count(s1.key())) continue;
        long score = 0;
        for (const Move& m2 : get_moves(s1)) score += get_moves(apply(s1, m2)).size();
        if (score > best_score) { best_score = score; best = i; }
    }
    return best;
}

// ---- shared by strategy_1 and strategy_3: a simple preference order for moves
// rank 0: a card to the suits
// rank 1: a pile move that turns up a face-down card
// rank 2: anything else, except resetting the hand
// rank 3: reset the hand
int move_rank(const State& s, const Move& m) {
    if (m.kind == ACE_TO_SUIT || m.kind == CARD_TO_SUIT) return 0;
    if (m.kind == RESET) return 3;
    int c = m.card, col = s.col[c], w = s.where(c);
    if (col >= FIRST_PILE && col <= LAST_PILE && w > 1)
        for (int d = 0; d < NCARD; d++)
            if (s.col[d] == col && s.where(d) == w - 1 && !s.up(d)) return 1;
    return 2;
}

// number of face-down cards under card c in its pile (0 if c is not in a pile)
int face_down_under(const State& s, int c) {
    int col = s.col[c], n = 0;
    if (col < FIRST_PILE || col > LAST_PILE) return 0;
    for (int d = 0; d < NCARD; d++) if (s.col[d] == col && !s.up(d)) n++;
    return n;
}

// moves sorted by rank; within rank 1, the pile with the most face-down cards first;
// otherwise get_moves order is kept
std::vector<int> preferred_order(const State& s, const std::vector<Move>& moves) {
    std::vector<int> idx(moves.size()), key(moves.size());
    for (size_t i = 0; i < moves.size(); i++) {
        idx[i] = i;
        int r = move_rank(s, moves[i]);
        key[i] = r * 100 - (r == 1 ? face_down_under(s, moves[i].card) : 0);
    }
    std::stable_sort(idx.begin(), idx.end(), [&](int a, int b) { return key[a] < key[b]; });
    return idx;
}

// strategy_1: send cards to the suits at once; else turn up a face-down card
// (pile with the most face-down cards first); else strategy_0's rule.
// Moves back to a visited state are skipped, as in strategy_0.
int pick_preferences(const State& s, const std::vector<Move>& moves, const Visited& seen) {
    for (int i : preferred_order(s, moves)) {
        if (move_rank(s, moves[i]) > 1) break;
        if (!seen.count(apply(s, moves[i]).key())) return i;
    }
    return pick_lookahead2(s, moves, seen);
}

// strategy_2: strategy_0 looking one move further. Score of a move = number of
// three-move paths after it (strategy_0 counts two-move paths). Nothing else changes.
long count_paths(const State& s, int depth) {
    std::vector<Move> ms = get_moves(s);
    if (depth == 1) return ms.size();
    long n = 0;
    for (const Move& m : ms) n += count_paths(apply(s, m), depth - 1);
    return n;
}
int pick_lookahead3(const State& s, const std::vector<Move>& moves, const Visited& seen) {
    long best_score = 0; int best = -1;
    for (size_t i = 0; i < moves.size(); i++) {
        State s1 = apply(s, moves[i]);
        if (seen.count(s1.key())) continue;
        long score = count_paths(s1, 3);
        if (score > best_score) { best_score = score; best = i; }
    }
    return best;
}

// strategy_3: backtracking (depth-first) search. Try moves in preferred_order;
// at a dead end, undo the last move and try the next one. Never enter a state
// already examined in this game (64-bit hash of the state). Stops at a win, when
// every reachable state has been tried ("exhausted": the deal cannot be won under
// these rules), or after node_limit states ("limit": unknown).
// Undoing a move after seeing the card it turned up is like knowing the face-down
// cards: this answers "can this deal be won at all?", not "would a player win it?".
uint64_t state_hash(const State& s) {
    uint64_t h = 1469598103934665603ULL;                 // FNV-1a
    for (int c = 0; c < NCARD; c++) { h ^= (uint8_t)s.col[c]; h *= 1099511628211ULL; }
    for (int c = 0; c < NCARD; c++) { h ^= (uint8_t)s.pos[c]; h *= 1099511628211ULL; }
    return h;
}

long NODE_LIMIT = 2000000;
bool PROGRESS = true;          // print a line every PROGRESS_EVERY states
const long PROGRESS_EVERY = 250000;

Result play_search(const std::string& name, int seed, State s, std::ostream* trace) {
    sort_cols(s);
    struct Frame { State s; std::vector<Move> moves; std::vector<int> order; size_t next; };
    auto frame = [](const State& st) {
        Frame f{st, get_moves(st), {}, 0};
        f.order = preferred_order(st, f.moves);
        return f;
    };
    std::unordered_set<uint64_t> seen;
    std::vector<Frame> stack;
    stack.push_back(frame(s));
    seen.insert(state_hash(s));
    long nodes = 1;
    int best = s.score();
    std::string status = "exhausted";
    while (!stack.empty()) {
        Frame& f = stack.back();
        if (f.s.score() == NCARD) { status = "win"; break; }
        if (f.next >= f.order.size()) { stack.pop_back(); continue; }
        if (nodes >= NODE_LIMIT) { status = "limit"; break; }
        State s1 = apply(f.s, f.moves[f.order[f.next++]]);
        if (!seen.insert(state_hash(s1)).second) continue;
        nodes++;
        best = std::max(best, s1.score());
        if (PROGRESS && nodes % PROGRESS_EVERY == 0)
            std::fprintf(stderr, "  %s seed %d: %ld states, depth %zu, best score %d\n",
                         name.c_str(), seed, nodes, stack.size(), best);
        stack.push_back(frame(s1));
    }
    if (status == "win") {
        if (trace)
            for (size_t i = 0; i < stack.size(); i++)
                *trace << name << "," << seed << "," << i + 1 << ",\"" << stack[i].s.csv() << "\"\n";
        return {name, seed, NCARD, (int)stack.size() - 1, NCARD, status, nodes};
    }
    // no win: report the most cards on the suits seen anywhere in the search
    return {name, seed, best, 0, best, status, nodes};
}

struct Strategy {
    std::string name, desc;
    std::function<Result(int seed, const State& deal, std::ostream* trace)> play;
};

const std::vector<Strategy> STRATEGIES = {
    {"strategy_0",
     "R v30 (one_step.R): play the move with the most two-move paths after it; "
     "skip moves back to a visited state; ties go to the first move; stop when every score is 0.",
     [](int seed, const State& d, std::ostream* t) { return play_stepwise("strategy_0", pick_lookahead2, seed, d, t); }},
    {"strategy_1",
     "Simple preferences: send a card to the suits at once; else turn up a face-down card "
     "(pile with the most face-down cards first); else strategy_0's rule.",
     [](int seed, const State& d, std::ostream* t) { return play_stepwise("strategy_1", pick_preferences, seed, d, t); }},
    {"strategy_2",
     "Deeper lookahead: strategy_0, but a move's score is the number of three-move paths after it "
     "(strategy_0 counts two-move paths).",
     [](int seed, const State& d, std::ostream* t) { return play_stepwise("strategy_2", pick_lookahead3, seed, d, t); }},
    {"strategy_3",
     "Backtracking search: depth-first, moves tried in strategy_1's order, undo at dead ends, "
     "never revisit a state. Answers whether the deal can be won at all (undo reveals face-down cards).",
     [](int seed, const State& d, std::ostream* t) { return play_search("strategy_3", seed, d, t); }},
};

const Strategy* find_strategy(const std::string& name) {
    for (const Strategy& st : STRATEGIES) if (st.name == name) return &st;
    return nullptr;
}

// ---------------------------------------------------------------- input
// read a deal file (header row, then seed + 104 numbers per row)
std::vector<std::pair<int, State>> read_deals(const std::string& file) {
    std::ifstream in(file);
    if (!in) { std::cerr << "cannot open " << file << "\n"; std::exit(1); }
    std::vector<std::pair<int, State>> deals;
    std::string line;
    std::getline(in, line);   // header
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string tok;
        std::vector<int> v;
        while (std::getline(ss, tok, ',')) v.push_back(std::atoi(tok.c_str()));
        if (v.size() != 105) continue;
        State s;
        for (int c = 0; c < NCARD; c++) { s.col[c] = v[1 + c]; s.pos[c] = v[1 + NCARD + c]; }
        deals.push_back({v[0], s});
    }
    return deals;
}

// "0-20,222,317" -> {0..20, 222, 317}
std::unordered_set<int> parse_seeds(const std::string& spec) {
    std::unordered_set<int> out;
    std::stringstream ss(spec);
    std::string part;
    while (std::getline(ss, part, ',')) {
        size_t d = part.find('-');
        int lo = std::stoi(part.substr(0, d));
        int hi = d == std::string::npos ? lo : std::stoi(part.substr(d + 1));
        for (int i = lo; i <= hi; i++) out.insert(i);
    }
    return out;
}

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out; std::stringstream ss(s); std::string x;
    while (std::getline(ss, x, sep)) if (!x.empty()) out.push_back(x);
    return out;
}

// ---------------------------------------------------------------- main
int main(int argc, char** argv) {
    std::string deals_file = "seed_ch.csv", out_file = "results.csv", trace_file,
                strategy_arg = "strategy_0", seeds_arg;
    bool quiet = false, append = false;
    int jobs = 1;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--list") {
            for (const Strategy& st : STRATEGIES) std::printf("%-12s %s\n", st.name.c_str(), st.desc.c_str());
            return 0;
        }
        if (a == "--quiet") { quiet = true; continue; }
        if (a == "--append") { append = true; continue; }
        if (i + 1 >= argc) { std::cerr << "missing value after " << a << "\n"; return 1; }
        std::string v = argv[++i];
        if      (a == "--deals")    deals_file = v;
        else if (a == "--out")      out_file = v;
        else if (a == "--trace")    trace_file = v;
        else if (a == "--strategy") strategy_arg = v;
        else if (a == "--seeds")    seeds_arg = v;
        else if (a == "--jobs")     jobs = std::max(1, std::stoi(v));
        else if (a == "--node-limit") NODE_LIMIT = std::stol(v);
        else { std::cerr << "unknown option " << a << "\n"; return 1; }
    }

    std::vector<const Strategy*> chosen;
    if (strategy_arg == "all") for (const Strategy& st : STRATEGIES) chosen.push_back(&st);
    else for (const std::string& n : split(strategy_arg, ',')) {
        const Strategy* st = find_strategy(n);
        if (!st) { std::cerr << "unknown strategy " << n << " (try --list)\n"; return 1; }
        chosen.push_back(st);
    }

    auto deals = read_deals(deals_file);
    if (!seeds_arg.empty()) {
        std::unordered_set<int> want = parse_seeds(seeds_arg);
        std::vector<std::pair<int, State>> kept;
        for (auto& d : deals) if (want.count(d.first)) kept.push_back(d);
        if (kept.size() < want.size())
            std::cerr << "note: " << want.size() - kept.size() << " requested seeds are not in " << deals_file << "\n";
        deals = kept;
    }

    PROGRESS = !quiet;
    std::ofstream out(out_file, append ? std::ios::app : std::ios::trunc);
    if (!append) out << "strategy,seed,score,moves,finished,won,search,nodes\n";
    std::ofstream tr;
    bool tracing = !trace_file.empty();
    if (tracing) { tr.open(trace_file, append ? std::ios::app : std::ios::trunc); if (!append) tr << "strategy,seed,i,ch\n"; }

    for (const Strategy* st : chosen) {
        auto t0 = std::chrono::steady_clock::now();
        // games are independent: play them on `jobs` threads, write in seed order
        std::vector<Result> results(deals.size());
        std::vector<std::string> traces(deals.size());
        std::atomic<size_t> next{0};
        std::mutex io;
        std::vector<char> done(deals.size(), 0);
        size_t written = 0;
        int wins = 0, limits = 0, played = 0;
        auto write_ready = [&]() {          // call with io locked
            while (written < deals.size() && done[written]) {
                const Result& r = results[written];
                bool won = r.finished == NCARD;
                wins += won; limits += (r.search == "limit");
                out << r.strategy << "," << r.seed << "," << r.score << "," << r.moves << ","
                    << r.finished << "," << (won ? 1 : 0) << "," << r.search << "," << r.nodes << "\n";
                if (tracing) { tr << traces[written]; traces[written].clear(); }
                written++;
            }
            out.flush();
        };
        auto worker = [&]() {
            for (size_t k; (k = next++) < deals.size();) {
                auto g0 = std::chrono::steady_clock::now();
                std::ostringstream t;
                results[k] = st->play(deals[k].first, deals[k].second, tracing ? &t : nullptr);
                if (tracing) traces[k] = t.str();
                const Result& r = results[k];
                std::lock_guard<std::mutex> lock(io);
                done[k] = 1;
                write_ready();
                if (!quiet) {
                    double g = std::chrono::duration<double>(std::chrono::steady_clock::now() - g0).count();
                    played++;
                    std::fprintf(stderr, "[%d/%zu] %s seed %d: score %d, finished %d%s, %d moves",
                                 played, deals.size(), st->name.c_str(), r.seed, r.score, r.finished,
                                 r.finished == NCARD ? " WIN" : "", r.moves);
                    if (r.search != "-") std::fprintf(stderr, ", search %s after %ld states", r.search.c_str(), r.nodes);
                    std::fprintf(stderr, ", %.2fs\n", g);
                }
            }
        };
        std::vector<std::thread> pool;
        for (int j = 0; j < jobs; j++) pool.emplace_back(worker);
        for (auto& th : pool) th.join();

        double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        std::printf("%s: games=%zu wins=%d (%.1f%%)", st->name.c_str(), deals.size(), wins,
                    100.0 * wins / std::max<size_t>(deals.size(), 1));
        if (limits) std::printf(" unresolved=%d", limits);
        std::printf(" time=%.1fs\n", sec);
        std::fflush(stdout);
    }
}
