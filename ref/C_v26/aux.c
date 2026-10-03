/* ############################################################################### */


int sign(int x) {
    return (x > 0) - (x < 0);
}


/*
We need a routine to strip of the first element of a comma-separated string.
When we read in lines from "seed_ch.csv", the first element is the row number, which we discard.
The remaining elements are the 52+52 columns of the state, which we want to keep. 
*/
char* strip_first_element(char* str) {
    char* first_comma = strchr(str, ','); // Find the first occurrence of ','
    if (first_comma != NULL) {
        return first_comma + 1; // Return a pointer to the character after the comma
    } else {
        return str; // No comma found, return the original string
    }
}


/*
Load state character array from a file
Input: 
 - desired row number 
 - target string pointer
Output:
 - 0 if success
 - 1 if fail 
The routine keeps reading until it finds the matching row number
Then loads the string with the contents
*/
int load_line(int n, char* ch) {
    FILE *fp;
    char line[CHMAX];
    char *filename = "seed9_ch.csv"; 
    int num_lines = 0; // To keep track of how many rows are read

    // Open the file in read mode
    fp = fopen(filename, "r");
    // Check if the file was opened successfully
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }
    // Read a line using fscanf
    // %255[^\n] reads up to 255 characters until a newline is encountered
    // %*c consumes the newline character without storing it
    while (fscanf(fp, "%511[^\n]%*c", line) == 1) {
        // skip the first row (column labels)
        // rows labeled starting from n=0
        // so we need to compare num_lines to n+1
        if (num_lines == n+1) {
            // printf("Read line: %s\n", line);
            strncpy(ch, strip_first_element(line), CHMAX - 1); // -1 for null terminator space
            ch[CHMAX - 1] = '\0'; // Ensure null termination
            // printf("Read line: %s\n", ch);
        }
        num_lines += 1;
    }
    // Close the file
    fclose(fp);
    return 0;
}


/*
Converts a comma-separated character string to an array of integers
The function `strtok(ch,",")` MODIFIES the string ch
Specifically it removes the ","
If we want to use ch after strtok, we better make a copy!
*/
int get_s_from_ch(char* ch, int s[]) {
    char *token; 
    char dummy[CHMAX]; 
    int count = 0; 

    strcpy(dummy,ch); 
    // Get the first token
    token = strtok(dummy, ",");
    // Loop through the rest of the tokens
    while (token != NULL) {
        // Convert the token string to an integer
        s[count] = atoi(token);
        count++;
        // Get the next token
        token = strtok(NULL, ",");
    }
    return 0;
}



/*
Converts a comma-separated character string to an array of integers
*/
int get_ch_from_s(int s[], char* ch) {
    int i; 
    char buffer[4]; 

    strcpy(ch, "\0");
    for (i=0; i<SMAX; i++) {
        snprintf(buffer, sizeof(buffer), "%d", s[i]);
        strcat(ch, buffer);
        if (i < SMAX-1) strcat(ch, ",");
    }
//    strcat(ch, "\0");
    return 0;
}




/*
Input: 
 - card k, where abs(k) = 1..52 (signed)
Output:
 - label, AC..KS, possibly with "-" prefix
suit = 0..3, face = 0..12
*/
int get_label(int k, char* label) {
    int suit, face, i, j; 
    char suit_label[4][2] = {"C","D","H","S"}; 
    char face_label[13][2] = {"A","2","3","4","5","6","7","8","9","T","J","Q","K"}; 
    if (k < 0) {
        j = sign(k);
        i = (k/j)-1; // 
        suit = i /13; 
        face = i % 13;
        strcpy(label, "-");
        strcat(label, face_label[face]);
        strcat(label, suit_label[suit]);        
    } else if (k == 0) {
        strcpy(label, " ");
    } else {
        i = k-1; // 
        suit = i /13; 
        face = i % 13;
        strcpy(label, face_label[face]);
        strcat(label, suit_label[suit]);        
    }
    return 0; 
}


/*
Input: 
 - state ch
Output:
 - state matrix mat (NROW = 24, NCOL=12)
the matrix is flat (1D, not 2D), so we need to indicate 
how to unfold (row,col)
Consider when NCOL=4 and NROW=2, then i = 0..7
0 1 2 3
4 5 6 7
i = row*NCOL + col
*/
int load_mat(char* ch, int mat[]) {
    int i,j,k,row,col;
    int s[SMAX]; // Declare an array to store integers

    // load the integer array corresponding to ch
    get_s_from_ch(ch,s); 
    // initialize mat
    for (i=0; i<NROW*NCOL; i++) {
        mat[i] = 0; 
    }
    // now we'll scan through arr to place each card in mat
    for (i=0; i<CMAX; i++) {
        col = s[i]-1; // the columns are unit-based 
        j = sign(s[i+CMAX]); 
        row = s[i+CMAX]/j-1; // the positions are unit-based 
        k = i+1; 
        mat[row*NCOL+col] = k*j; 
    }
    return 0; 
}


/*
Input: 
 - state ch
Output:
 - state matrix mat (NROW = 24, NCOL=12)
the matrix is flat (1D, not 2D), so we need to indicate 
how to unfold (row,col)
Consider when NCOL=4 and NROW=2, then i = 0..7
0 1 2 3
4 5 6 7
i = row*NCOL + col
*/
int show_mat(int mat[]) {
    int row,col,i,j;
    char label[3]; 

//    printf("...1...2...3...4...5...6...7...8...9..10..11\n");
    printf("   1   2   3   4   5   6   7   8   9  10  11\n");
    printf("...C...D...H...S..P1..P2..P3..P4..P5..P6..P7..ZZ  pos\n");
    for (row=0; row<NROW; row++) {
        for (col=0; col<NCOL; col++) {
            i = row*NCOL+col; 
            switch (XRAY) {
                case 0: // show player view
                    j = sign(mat[i]); 
                    if (j < 0) {
                        printf("%4s","-"); 
                    } else {
                        get_label(mat[i],label);
                        printf("%4s",label); 
                    }
                    break;
                case 1: // show all labels
                    get_label(mat[i],label);
                    printf("%4s",label); 
                    break;
                default: // show numbers 
                    printf("%4d",mat[i]); 
                    break;
            }
        }
        printf("   %2d\n",row+1); 
    }  
    return 0;   
}



// return the first array sorted
// return the second array in the same sequence as first array
void bubbleTwo(int arr[], int arr2[], int n) {
    int i, j, temp, temp2;

    for (i=0; i<n-1; i++) {
        for (j=0; j<n-1-i; j++) {
            if (arr[j] > arr[j+1]) {
                // Swap elements
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
                temp2 = arr2[j];
                arr2[j] = arr2[j+1];
                arr2[j+1] = temp2;
            }
        }
    }
}




/*
Input: 
 - state s
Output:
 - state s, now with columns sorted
sort the columns such that two states can be compared.
need to sort only the piles, not suits not hand
this means we change s_col, but not s_pos*s_sign
any unique sort method is acceptable.
want to sort in a way that is visually pleasing
want to sort is a way that is relatively stable
try increasing count, then id of top card
*/
int sort_s(int s[]) {
    int pile_cnt[7];
    int pile_top[7];
    int val[7]; 
    int new[7]; 
    int i,j;
    char label[4]; 
//    int scol[CMAX];
//    int spos[CMAX];
    int s2[CMAX]; 


    // initialize s2
    for (j=0; j<CMAX; j++) {
//        scol[j] = s[j]; // existing the column of each card 
//        spos[j] = abs(s[j+CMAX]); // existing position of each card
        s2[j] = s[j]; 
    }

    for (i=0; i<7; i++) {
        pile_cnt[i] = 0; // initialize pile_cnt 
        pile_top[i] = 0; // initialize pile_top
        val[i] = 0; // initialize val
        new[i] = i; // initialize new (default)
        // now get the pile count
        for (j=0; j<CMAX; j++) {
            if (s[j]==i+5) pile_cnt[i]++; 
        }
        // with the pile count, find the top card
        for (j=0; j<CMAX; j++) {
            if ((s[j]==i+5) & (abs(s[j+CMAX])==pile_cnt[i])) {
                pile_top[i] = j+1; 
            }
        }
        val[i] = (pile_cnt[i]*53 + pile_top[i]); // we want to sort this ascending
    }


    // report the cards on top, the counts, and the score
    if (DEBUG > 2) {
        printf("labels:   ");
        for (i=0; i<7; i++) {
            get_label(pile_top[i],label);
            printf("%4s",label);
        }
        printf("\n");
        printf("pile_cnt: ");
        for (i=0; i<7; i++) {
            printf("%4d",pile_cnt[i]);
        }
        printf("\n");
        printf("val:      ");
        for (i=0; i<7; i++) {
            printf("%4d",val[i]);
        }
        printf("\n");
        printf("orig:     ");
        for (i=0; i<7; i++) {
            printf("%4d",new[i]);
        }
        printf("\n");
    }



    // now sort val
    bubbleTwo(val,new,7); 



    // report the cards on top, the counts, and the score
    if (DEBUG > 2) {
        printf("new:      ");
        for (i=0; i<7; i++) {
            printf("%4d",new[i]);
        }
        printf("\n");
    }



    for (i=0; i<7; i++) {
        for (j=0; j<CMAX; j++) {
            if (s[j]==new[i]+5) {
                s2[j] = i+5; 
            }
        }
    }

    // change just the columns
    for (j=0; j<CMAX; j++) {
        s[j] = s2[j]; 
    }

    return 0; 
}

