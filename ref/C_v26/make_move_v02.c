/* ############################################################################### */
/*
input: 
 - s : original state vector
 - from_id : id of anchor card, usually moving.  
    (array index is 0-based, from_id is 1-based)
 - to_col : the column anchor card moves to
 - to_pos : the position anchor card moves to
output:
 - s2 : the new state vector
Todo: 
 - [x] When moving cards to suits, show them in the suits
 - [ ] When moving cards to piles, show them on the piles
*/
int say_move(int s[], int from_id, int to_col, int to_pos, int s2[]) {
    int scol[CMAX];
    int spos[CMAX];
    int ssign[CMAX];
    int s2col[CMAX];
    int s2pos[CMAX];
    int s2sign[CMAX];
    int from_col, from_pos; 
    int pat[NROW]; 
    int hand_cnt; 
    bool reg_hand[CMAX]; 
    int i,j; 


    hand_cnt = 0; 
    for (i=0; i<CMAX; i++){ // loop through cards
        scol[i] = s[i]; // existing the column of each card 
        spos[i] = abs(s[i+CMAX]); // existing position of each card
        ssign[i] = sign(s[i+CMAX]); // existing sign of each card
        reg_hand[i] = (scol[i] == 12); // is this card in the hand?
        if (reg_hand[i]) hand_cnt++;
        s2col[i] = scol[i]; 
        s2pos[i] = spos[i]; 
        s2sign[i] = ssign[i]; 
    }

    from_col = scol[from_id-1]; 
    from_pos = spos[from_id-1]; 

    // moves between the piles can affect more than one card
    if ((from_col < 12) & (to_col > 4)) {
        for (i=0; i<CMAX; i++){ // loop through cards
            if ((scol[i] == from_col) & (spos[i] >= from_pos)) {
                s2col[i] = to_col; 
                s2pos[i] = to_pos + spos[i] - spos[from_id-1]; 
                s2sign[i] = 1; 
            }
            if ((scol[i] == from_col) & (spos[i] == from_pos-1)) {
                s2sign[i] = 1; 
            }
        }
    } else if (from_col == 12) {
        for (i=0; i<CMAX; i++){ // loop through cards
            if ((scol[i] == from_col) & (spos[i] < from_pos)) {
                s2sign[i] = -1; 
            }
            if ((scol[i] == from_col) & (spos[i] == from_pos-1)) {
                s2sign[i] = 1; 
            }
            if ((scol[i] == from_col) & (spos[i] == from_pos)) {
                s2col[i] = to_col; 
                s2pos[i] = to_pos; 
                s2sign[i] = 1; 
            }
            if ((scol[i] == from_col) & (spos[i] > from_pos)) {
                s2pos[i] -= 1; 
            }
        }        
    }

    // if reset the hand, change just face not pos or col
    if ((from_col == to_col) & (from_pos == to_pos)) {
        // define the correct pattern in the hand
        for (i=0; i<hand_cnt; i++) {
            if (i%3 == 0) pat[i] = -1; 
            if (i%3 == 1) pat[i] = -1; 
            if (i%3 == 2) pat[i] = 1;
            if (i == hand_cnt - 1) pat[i] = 1; 
        }
        // now impose the pattern
        for (i=0; i<CMAX; i++) {
            if (reg_hand[i]) {
                j = spos[i]-1; 
                s2sign[i] = pat[j]; 
            }
        }
    }

    for (i=0; i<CMAX; i++){ // loop through cards
        s2[i] = s2col[i]; 
        s2[i+CMAX] = s2pos[i]*s2sign[i];
    }


    // ####################################################################################
    return 0; 
}