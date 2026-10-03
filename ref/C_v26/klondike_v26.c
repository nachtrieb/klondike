#include <stdio.h>
#include <string.h> // strcpy, strtok, strchr
#include <stdlib.h> // atoi, system
#include <stdbool.h> // bool

#define CMAX 52     // max index in a deck of Cards
#define SMAX 2*CMAX // max index in a State
#define CHMAX 512   // 104*(1+1+2) = 408; max length of CHaracter state
#define NROW 24     // max rows of matrix state
#define NCOL 12     // max cols of matrix state
#define NMAT NROW*NCOL // length of unfolded of matrix
#define XRAY 0      // 0 = player view, 1 = full view, 2 = numbers
#define DEBUG 2
#define MOVEMAX 1000

#include "aux.c"
#include "get_move_v03.c"
#include "say_move_v02.c"
#include "aux2.c"

/*

_v26
- [x] specify starting seed.  We read from a file for simplicity and 
      to be able to reproduce bugs found in testing. 
- [x] don't repeat moves
- [ ] other debug

To avoid repeating moves: 
- incorporate into the routine get_possible_moves
- make a list of the previous moves occupied (vector of ch)
- for each identified move, make a list of see_move, what it _would be_
- check each _would be_ against historical, eliminate duplicates
- return only the list of legitimate non-duplicated moves

Basic philosophy stays the same
- that we've identified all the moves
- that when we make a move, the right transformation happens
- develop the mechanisms for scoring
- testing whether we've won a game (final state, is winning state)
- testing whether we've lost a game (final state, is not winning state)

We have a sequence of states stored in ch
Each state is connected to the immediately prior by a valid move
A full game consists of a sequence of states

_v25
nearly complete working example.  
*/



/* ############################################################################### */
int main() {
    char ch[MOVEMAX][CHMAX]; 
    int s[SMAX],s2[SMAX]; 
    int i;
    int mat[NMAT]; 
    int moves[100][4]; // we'll modify this with get_move()
    int move_cnt; // we'll modify this with get_move()
    char label[3]; 
    int from_id, to_col, to_pos; 
    int pick; 
    int seed; 
    int move_tot = 0; 


    // initialize the game history
    for (i=0; i<MOVEMAX; i++) strcpy(ch[0], " ");

    // load the starting move
    system("clear"); 
    printf("\nPick starting seed (0..9): "); // Prompt the user for input
    scanf("%d", &seed);  
    load_line(seed,ch[move_tot]);

    // now start play
    printf("original state\n");
    display_current_state(ch[move_tot]); 
    get_possible_moves(ch[move_tot],moves,&move_cnt); 

    do {
        show_possible_moves(moves,move_cnt); 
        pick_move(move_cnt,&pick); 
        if (pick >= 0) {
            make_move(ch[move_tot],moves,pick,ch[move_tot+1]);
            move_tot++; 
            //strcpy(ch[0], ch[1]);
            sort_cols(ch[move_tot]);
            display_current_state(ch[move_tot]); 
            get_possible_moves(ch[move_tot],moves,&move_cnt);
            get_new_moves(ch,move_tot,moves,&move_cnt);              
        }
    } while ((pick >= 0) & (move_cnt>0)); 

    if (move_cnt==0) printf("Game over!\n"); 

    return 0;
}


