################################################
# Finding the moves
# There are five types of move we'll look for: 
#     description	    move count
# a. 	to suit	        single
# b. 	king to empty	  single
# c. 	pile to pile	  multi
# d. 	hand to pile	  single
################################################
get_move <- function(s, debug=0){
  if (debug > 2) browser()  
  
  # basic definitions
  s_col <- s[1:52]
  s_pos <- abs(s[53:104])
  s_sign <- sign(s[53:104])
  
  # find the cards in the hand
  reg_hand <- (s_col == 12) 
  hand <- deck_id[reg_hand] # id of the hand cards
  
  # find the face-up cards in the hand
  reg_hand_up <- (s_col == 12) & (s_sign>0)
  hand_up <- deck_id[reg_hand_up] # id of the hand up cards
  
  # find the face-up cards in the pile
  reg_pile_up <- (s_col > 4) & (s_col < 12) & (s_sign>0)
  pile_up <- deck_id[reg_pile_up] # id of the pile up cards
  
  # find the pile count and top card in each pile
  cnt <- rep(0,7) # starting pile count
  pile_top <- rep(NA,7)
  for (i in 1:7) {
    cnt[i] = sum(s_col == (i+4))
    reg <- (s_col == (i+4)) & (s_pos==cnt[i]) # find the top card
    if (sum(reg)>0) pile_top[i] <- deck_id[reg]
  }
  reg_pile_top <- !is.na(pile_top)
  pile_top <- pile_top[reg_pile_top] # id of the top pile cards
  n_top <- length(pile_top)
  
  
  if (debug > 2) browser()  
  
  #####
  # Now get the moves
  #####
  move <- data.frame()
  m <- 0
  
  ## define move to reset the hand.  This is an option
  # if the face cards in the hand have changed from nominal
  
  # find all the cards in the hand
  hand_pos <- s_pos[hand]
  
  x <- sort(hand_pos,index.return=TRUE)
  ii <- x$ix
  
  # find the correct hand pattern
  n <- sum(reg_hand) # number of cards in hand
  m <- as.integer(n/3)
  r <- n - m*3
  pat3 <- c(-1,-1,1)
  pat_m <- rep(pat3,m)
  pat_r <- numeric()
  if (r > 0) pat_r <- pat3[seq(1,r)]
  pat <- c(pat_m,pat_r)
  if (n > 0) pat[n] <- 1
  
  already_reset <- s_sign[hand[ii]] == pat
  if (!all(already_reset)) {
    pos_hand <- s_pos[hand_up]
    reg <- pos_hand == max(pos_hand)
    from_id <- hand_up[reg]
    to_id <- from_id
    to_col <- s[to_id]
    to_pos <- s[to_id + 52]
    this_move <- data.frame(kind = "z. reset hand"
      ,from_id = from_id,to_col = to_col,to_pos = to_pos)
    m <- m+1
    move <- bind_rows(move,this_move)
  } 
  
  if (debug > 2) browser()  
  
  ## move the top pile or hand to suits (ace or not)
  all_from_id <- c(pile_top,hand_up)
  n <- length(all_from_id)
  if (n > 0){
    for (i in 1:n){
      from_id <- all_from_id[i]
      to_id <- from_id-1 # provisional
      to_col <- deck_suit[from_id]
      to_pos <- deck_face[from_id]
      if (from_id %in% aces) {
        this_move <- data.frame(kind = "a. ace to suit"
          ,from_id = from_id,to_col = to_col,to_pos = to_pos)
        m <- m+1
        move <- bind_rows(move,this_move)
      } else if (!(from_id %in% aces) & (s[to_id] < 5)) {
        this_move <- data.frame(kind = "b. non-ace to suit"
          ,from_id = from_id,to_col = to_col,to_pos = to_pos)
        m <- m+1
        move <- bind_rows(move,this_move)
      }
    }
  }
  
  if (debug > 2) browser()  
  
  ## move king to an empty pile
  reg_open <- cnt == 0
  n_open <- sum(reg_open) # the count of empty columns
  reg_king <- 
    ((s_col[king] > 4) & (s_col[king] < 12) & (s_pos[king]>1) & (s_sign[king]>0)) | 
    ((s_col[king] == 12) & (s_sign[king]>0))
  n_king <- sum(reg_king) # the count of up kings not in suits
  if ((n_open > 0) & (n_king > 0)) {
    all_from_id <- king[reg_king]
    to_cols <- piles[reg_open]
    for (i in 1:n_king) {
      for (j in 1:n_open) {
        from_id <- all_from_id[i]
        to_col <- to_cols[j]
        to_pos <- 1
        this_move <- data.frame(kind = "c. king to empty"
          ,from_id = from_id,to_col = to_col,to_pos = to_pos)
        m <- m+1
        move <- bind_rows(move,this_move)
      }
    }
  }
  
  if (debug > 2) browser()  
  
  all_up <- c(pile_up,hand_up)
  # move to top pile
  if (n_top > 0) {
    for (i in 1:n_top) {
      to_id <- pile_top[i]
      to_col <- s[to_id]
      to_pos <- s[to_id+52]+1
      reg_from <- 
        (deck_face[all_up] == (deck_face[to_id]-1)) & 
        (deck_color[all_up] == (1-deck_color[to_id])) & 
        (s[all_up] != to_col)
      n_from <- sum(reg_from) 
      if (n_from > 0) {
        all_from_id <- all_up[reg_from]
        for (j in 1:n_from) {
          from_id <- all_from_id[j]
          this_move <- data.frame(kind = "d. to pile"
            ,from_id = from_id,to_col = to_col,to_pos = to_pos)
          m <- m+1
          move <- bind_rows(move,this_move)
        }  
      }
    }
  }
  
  if (debug > 0) browser()  
  return(move)
}
