################################################
# Making the move
# given single (1) anchor FROM and TO.
# 1. Find all (1+) card_id of FROM
# 2. Find all (1+) col and pos of TO
# 3. Change all (1+) col and pos of FROM to TO
# If FROM col is hand, then
### Treat Hand
# 4. Find all (0+) card_id in hand after FROM; pos--
# 5. Find all (0+) card_id in hand before FROM pos; face down. Card just below FROM pos face up
### Reset Hand
# 6. Turn all face down
# 7. Turn every third and last face up.
#
# description	    move count  treat hand?
# -----------     ----------  -----------
# pile to suit    single      N
# pile to pile    multi       N
# king to empty   single      ? (Y if from hand)
# hand to suit    single      Y
# hand to pile    single      Y
#
################################################
make_move <- function(s, from_id, to_col, to_pos, debug=0){
  if (debug > 2) browser()
  
  s_col <- s[1:52]
  s_pos <- abs(s[53:104])
  s_sign <- sign(s[53:104])
  
  # set the initial state
  s2_col <- s_col
  s2_pos <- s_pos
  s2_sign <- s_sign
  
  from_col <- s_col[from_id]
  from_pos <- s_pos[from_id]
  
  # by default, only the anchor card moves
  from_ids <- from_id
  to_cols <- to_col
  to_poss <- to_pos
  
  if ((from_col < 12) & (to_col > 4)) {
    reg <- (s_col == from_col) & (s_pos >= from_pos)
    from_ids <- deck_id[reg]
    to_cols <- rep(to_col,sum(reg))
    to_poss <- to_pos + s_pos[reg] - s_pos[from_id]
  }
  
  # now make the changes! 
  s2_col[from_ids] <- to_cols
  s2_pos[from_ids] <- to_poss
  s2_sign[from_ids] <- 1 # should already have been up
  
  # Now turn the cards up
  if ((from_col < 12) & (from_pos > 1)) { # from pile
    reg <- (s_col == from_col) & (s_pos == from_pos-1)
    up_ids <- deck_id[reg]
    s2_sign[up_ids] <- 1
  } else if (from_col == 12) { # from hand
    reg <- (s_col == 12) & (s_pos < from_pos)
    if (sum(reg) > 0) {
      before_ids <- deck_id[reg]
      s2_sign[before_ids] <- -1
    }
    reg <- (s_col == 12) & (s_pos == from_pos-1)
    if (sum(reg) > 0) {
      below_ids <- deck_id[reg]
      s2_sign[below_ids] <- 1
    }
    reg <- (s_col == 12) & (s_pos > from_pos)
    if (sum(reg) > 0) {
      after_ids <- deck_id[reg]
      s2_pos[after_ids] <- s2_pos[after_ids] - 1 # shift down
    }
  }
  
  # if reset the hand, change just face not position or column
  if ((from_col==to_col) & (from_pos==to_pos)) {
    
    # find all the cards in the hand
    reg_hand <- (s2_col == 12) 
    hand_id <- deck_id[reg_hand] # id of the hand up cards; sorted!
    hand_pos <- s2_pos[reg_hand]
    
    x <- sort(hand_pos,index.return=TRUE)
    
    # now make the changes! 
    n <- sum(reg_hand) # number of cards in hand
    m <- as.integer(n/3)
    r <- n - m*3
    pat3 <- c(-1,-1,1)
    pat_m <- rep(pat3,m)
    pat_r <- numeric()
    if (r > 0) pat_r <- pat3[seq(1,r)]
    pat <- c(pat_m,pat_r)
    if (n > 0) pat[n] <- 1
    
    s2_sign[hand_id[x$ix]] <- pat
  } 
  
  s2 <- c(s2_col,s2_pos*s2_sign)
  if (debug > 0) browser()
  return(s2)
}
