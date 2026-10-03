int sort_cols(char* ch){
    int s[SMAX]; 

    get_s_from_ch(ch,s); 
    sort_s(s); 
    get_ch_from_s(s,ch); 
    return 0;
}


int display_current_state(char* ch) {
    int s[SMAX]; 
    int mat[NMAT]; 

    load_mat(ch,mat); 
    show_mat(mat); 
    return 0;
}


int get_possible_moves(char* ch, int (*moves)[4], int *move_cnt_ptr) {
    int move_cnt;
    int s[SMAX]; 

    get_s_from_ch(ch,s); 
    get_move(s,moves,&move_cnt);
    *move_cnt_ptr = move_cnt; 
    return 0;
}





int show_possible_moves(int (*moves)[4], int move_cnt) {
    int i; 
    char label[3]; 
    int from_id, to_col, to_pos; 

    for (i=0; i<move_cnt; i++) {
        from_id = moves[i][1];
        to_col = moves[i][2];
        to_pos = moves[i][3];
        get_label(from_id,label);
        printf("i=%2d move %s to_col=%2d to_pos=%2d type=%2d\n"
        ,i,label,moves[i][2],moves[i][3],moves[i][0]);
    }
    return 0;
}


int pick_move(int move_cnt, int *pick_ptr) {
    int pick = move_cnt; 
    bool valid; 

    do {
        printf("\nPick a move (neg to quit): "); // Prompt the user for input
        scanf("%d", &pick);  
        valid = (pick < move_cnt);
        if (!valid) {
            printf("You entered: %d\n", pick); 
            printf("Pick a number between 0 and %d\n", move_cnt-1); 
        }
    } while (!valid);
    *pick_ptr = pick; 
    return 0; 
}


int make_move(char* ch0, int (*moves)[4], int pick, char* ch1){
    int s[SMAX],s2[SMAX]; 
    int from_id, to_col, to_pos; 

    get_s_from_ch(ch0,s); 
    from_id = moves[pick][1];
    to_col = moves[pick][2];
    to_pos = moves[pick][3];
    say_move(s,from_id,to_col,to_pos,s2); 
    get_ch_from_s(s2,ch1); 
    return 0; 
}


/*
Test which of the proposed moves is new.  
A move is new if the resulting state chtest is NOT in the list of states 
previously visited ch[MOVEMAX]
We then want to return a reduced set of moves (just the new ones), 
and a reduced move_cnt
*/
int get_new_moves(char ch[][CHMAX], int move_tot, int (*moves)[4], int *move_cnt_ptr) {
    int move_cnt;
    int s[SMAX]; 
    int i,j,k; 
    char chtest[CHMAX]; 
    bool new; 
    int diff; 
    int tmpmoves[100][4];
    int new_move_cnt;

    // initialize tmpmoves
    for (i=0;i<100;i++) for (k=0;k<4;k++) tmpmoves[i][k] = 0; 
    new_move_cnt = 0; 
    move_cnt = *move_cnt_ptr;
    for (j=0; j<move_cnt; j++) {
        make_move(ch[move_tot],moves,j,chtest);
        sort_cols(chtest);
        new = 1; 
        for (i=0; i<move_tot; i++) {
            // are the strings different? 
            diff = strcmp(ch[i],chtest);
            new *= (diff != 0);
        }
        if (new==1) {
            for (k=0;k<4;k++) {
                tmpmoves[new_move_cnt][k] = moves[j][k]; 
            }
            new_move_cnt++; 
        }
    }
    // initialize moves
    for (i=0;i<100;i++) for (k=0;k<4;k++) moves[i][k] = 0; 
    // back fill with just the new moves
    for (i=0;i<new_move_cnt;i++) for (k=0;k<4;k++) moves[i][k] = tmpmoves[i][k]; 
    *move_cnt_ptr = new_move_cnt; 
    return 0;
}

