/* ############################################################################### */
/*
input: 
-s state
output:
moves; 
  matrix whose columns are 
    1. kind, 
    2. from_id, 
    3. to_col, 
    4. to_pos; 
  integer with vector length
We will check for the following possible moves: 
   [x] 5. reset the hand
   [x] 1. move top pile ace to suits
   [x] 2. top pile non-ace to suits
   [x] 3. move king to an empty pile
   [x] 4. move substack to top pile
*/
int get_move(int s[], int (*moves)[4], int *move_cnt_ptr) {
    int scol[CMAX];
    int spos[CMAX];
    int ssign[CMAX];
    bool reg_hand[CMAX]; 
    bool reg_hand_up[CMAX]; 
    bool reg_pile_up[CMAX]; 
    bool reg_ace[CMAX];
    bool reg_king[CMAX];
    int pile_cnt[7]; 
    int pile_top[7]; 
    int ntop; 
    int i,j,k; 
    char label[3]; 
    int pat[NROW]; 
    int hand_cnt; 
    bool this_reset, already_reset; 
    int from_id,from_col,from_pos; 
    int to_id,to_col,to_pos; 
    int max_pos_hand_up; 
    int suit,face; 
    int suit_j,face_j; 
    const int suitcolor[] = {0,1,1,0}; // 0=black, 1=red.  Maps to {C,D,H,S}
    int move_cnt; 

    // ####################################################################################
    /* (0) initialization and basics
    */
    
    // initialize moves
    for (i=0; i<100; i++) {
        for (j=0; j<4; j++) {
            moves[i][j] = 0; 
        }
    }
    move_cnt = 0; 
    if (DEBUG > 2) printf("move_cnt reset %d\n",move_cnt); 

    // initialize pattern (down-down-up) for hand reset
    for (i=0; i<NROW; i++) { // there are at most NROW=24 cards in the hand
        pat[i] = 0; 
    }

    // first basic definitions
    // find cards in hand
    // find face-up cards in hand
    // find face-up cards in pile
    // find how many cards are in the hand

    hand_cnt = 0; 
    for (i=0; i<CMAX; i++){ // loop through cards
        scol[i] = s[i]; // existing the column of each card 
        spos[i] = abs(s[i+CMAX]); // existing position of each card
        ssign[i] = sign(s[i+CMAX]); // existing sign of each card
        reg_hand[i] = (scol[i] == 12); // is this card in the hand?
        if (reg_hand[i]) hand_cnt++; 
        reg_hand_up[i] = (scol[i] == 12) & (ssign[i] > 0); // in hand and up?
        reg_pile_up[i] = (scol[i] > 4) & (scol[i] < 12) & (ssign[i] > 0); // in piles and up?
        reg_ace[i] = (i%13 == 0); // is an ace?
        reg_king[i] = (i%13 == 12); // is a king?
    }

    // find the pile count 
    // find the top card in each pile
    ntop = 0; // initialize how many top cards there are
    for (j=0; j<7; j++) { // loop through piles
        pile_cnt[j] = 0; // initialize counts
        pile_top[j] = 0; // initialize which card is on top of each pile
        for (i=0; i<CMAX; i++) {
            if (scol[i] == j+5) pile_cnt[j]++; 
        }
        for (i=0; i<CMAX; i++) {
            if ((scol[i] == j+5) & (spos[i] == pile_cnt[j])) pile_top[j] = i+1; 
        }
        if (pile_top[j] > 0) ntop++; 
    }

    // ####################################################################################
    /* (5) 
    define the move to reset the hand. 
    this is an option if the face cards in the hand have changed from nominal
    first we find how many cards are in the hand
    then define the reset down-down-up pattern
    finally we compare with the face pattern
    */ 

    for (i=0; i<hand_cnt; i++) {
        if (i%3 == 0) pat[i] = -1; 
        if (i%3 == 1) pat[i] = -1; 
        if (i%3 == 2) pat[i] = 1;
        if (i == hand_cnt - 1) pat[i] = 1; 
    }

    // already_reset is true (1) if every one of the hand cards 
    // is up where supposed to 
    // is down where supposed to
    // logically this is an AND operation
    // initialize already_reset to TRUE
    already_reset = 1; 
    for (i=0; i<CMAX; i++) {
        if (reg_hand[i]) {
            j = spos[i]-1; 
            this_reset = (pat[j] == ssign[i]); 
            already_reset *= this_reset; 
        }
    }

    // if not already_reset, then resetting the hand is a possible move
    // the reset change only the face, not position or column
    // so from_col == to_col & from_pos == to_pos
    // for from_id we'll use the top-mose face-up card in the hand
    if (!already_reset) {
        from_id = 0; 
        from_col = 0; 
        from_pos = 0; 
        max_pos_hand_up = 0; 
        for (i=0; i<CMAX; i++) {
            if (reg_hand_up[i]) {
                j = spos[i]-1; 
                if (j > max_pos_hand_up) {
                    max_pos_hand_up = j;
                    from_id = i+1;
                    from_col = scol[from_id-1];
                    from_pos = spos[from_id-1];
                }
            }
        }
        to_id = from_id;
        to_col = from_col; 
        to_pos = from_pos; 
        moves[move_cnt][0] = 5; // reset hand
        moves[move_cnt][1] = from_id; 
        moves[move_cnt][2] = to_col; 
        moves[move_cnt][3] = to_pos;
        move_cnt += 1; 
        if (DEBUG > 2) printf("move_cnt %d\n",move_cnt); 

    }

    // ####################################################################################
    /* (1), (2) 
    We'll look at moving the top pile or hands to suits (ace or not)
    is the card the top card in one of the piles? 
        if the card is an ace, move it!
        if the card is not an ace, and the card below it is in one of the ace piles, move it!
    is the card in the hand and up? 
        if the card is an ace, move it!
        if the card is not an ace, and the card below it is in one of the ace piles, move it!
    */

    // first we'll loop through the piles
    // actually, we should loop through all the *cards*. 
    // if the card is the pile_top or hand_up then see about moving. 
    /*
        if card is in one of the piles AND is the top card of the pile 
        OR if card is in the hand and up
    */
    for (i=0; i<CMAX; i++) {
        k = 0; // default not a Kandidate
        k += 1*(reg_hand_up[i]); 
        for (j=0;j<7;j++){
            k += 1*(reg_pile_up[i] & (pile_top[j]==(i+1)));
        }
        if (k > 0) {
            from_id = i+1;
            from_col = scol[from_id-1];
            from_pos = spos[from_id-1];
            if (reg_ace[from_id-1]) { // it's an ace, move it directly
                to_id = from_id;
                i = from_id-1; 
                suit = i /13; 
                to_col = suit+1; 
                to_pos = 1; 
                moves[move_cnt][0] = 1; // move ace to piles
                moves[move_cnt][1] = from_id; 
                moves[move_cnt][2] = to_col; 
                moves[move_cnt][3] = to_pos;
                move_cnt += 1; 
                if (DEBUG > 2) printf("move_cnt %d\n",move_cnt); 

            } else if (scol[from_id-2] < 5) { // not an ace, but card below is in the ace piles
                to_id = from_id-1;
                i = from_id-1; 
                suit = i /13; 
                to_col = suit+1; 
                face = i %13;
                to_pos = face+1; // move to top of suit pile
                moves[move_cnt][0] = 2; // move non-ace to piles
                moves[move_cnt][1] = from_id; 
                moves[move_cnt][2] = to_col; 
                moves[move_cnt][3] = to_pos;
                move_cnt += 1; 
                if (DEBUG > 2) printf("move_cnt %d\n",move_cnt); 

            }
        }
    }


    // ####################################################################################
    /* (3) 
    move king to an empty pile (pile_cnt == 0)
    move if: 
    is king AND 
    (is in one of piles OR is in hand) AND
    is face-up AND
    one of the piles is empty
    */

    for (i=0; i<CMAX; i++){ // loop through cards
        k = 1*reg_king[i]; // has to be a King
        k *= 1*(scol[i]>4); // in one of piles or hand
        k *= ssign[i]; // has to be face-up
        for (j=0; j<7; j++) { // loop through piles
            if ((k>0) & (pile_cnt[j] == 0)) { 
                from_id = i+1; 
                to_col = j+5; 
                to_pos = 1; 
                moves[move_cnt][0] = 3; // move king to empty pile
                moves[move_cnt][1] = from_id; 
                moves[move_cnt][2] = to_col; 
                moves[move_cnt][3] = to_pos;
                move_cnt += 1; 
                if (DEBUG > 2) printf("move_cnt %d\n",move_cnt); 

            }
        }
    }

    // ####################################################################################
    /* (4) 
    move move to top pile
    move if: 
    one of the piles has a top card AND 
    if a card with the (opposite color and one lower face value) is in (the piles or hand) AND is up
    */

    for (i=0; i<CMAX; i++){ // loop through cards
        k = 1*(scol[i]>4); // in one of piles or hand
        k *= ssign[i]; // has to be face-up
        for (j=0; j<7; j++) { // loop through piles
            if ((k>0) & (pile_cnt[j] > 0)) {
                // pile_top[j] is the top card of pile j, non-zero
                suit = i /13;
                face = i %13;
                suit_j = (pile_top[j]-1) /13; 
                face_j = (pile_top[j]-1) %13;
                if ((face_j-face==1) & (scol[i]!=(j+5)) & (suitcolor[suit] == 1-suitcolor[suit_j])) {
                    from_id = i+1; 
                    to_col = j+5; 
                    to_pos = pile_cnt[j]+1; 
                    moves[move_cnt][0] = 4; // move king to empty pile
                    moves[move_cnt][1] = from_id; 
                    moves[move_cnt][2] = to_col; 
                    moves[move_cnt][3] = to_pos;
                    move_cnt += 1; 
                    if (DEBUG > 2) printf("move_cnt %d\n",move_cnt); 

                }
            }
        }
    }


    // ####################################################################################

    *move_cnt_ptr = move_cnt; 
    return 0; 
}