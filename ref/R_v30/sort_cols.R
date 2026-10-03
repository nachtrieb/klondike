# sort the columns such that two states can be compared. 
# need to sort only the piles, not suits not hand
# this means we change s_col, but not s_pos*s_sign
# any unique sort method is acceptable. 
# want to sort in a way that is visually pleasing
# want to sort is a way that is relatively stable
# try increasing count, then id of top card
sort_cols <- function(s,debug=0){
  if (debug > 2) browser()
  
  s_col <- s[1:52]
  s_pos <- abs(s[53:104])
  s_sign <- sign(s[53:104])
  
  # find the pile count and top card in each pile
  pile_cnt <- rep(0,7) # starting pile count
  pile_top <- rep(0,7)
  for (i in 1:7) {
    reg_pile <- s_col==piles[i]
    pile_cnt[i] = sum(reg_pile)
    reg <- reg_pile & (s_pos==pile_cnt[i]) # find the top card
    if (sum(reg)>0) pile_top[i] <- deck_id[reg]
  }
  # value; this is what we'll sort
  val <- pile_cnt*53 + pile_top
  val2 <- sort(val, index.return=TRUE) # x$ix is all we care about
  
  if (debug > 1) browser()
  
  piles2 <- val2$ix+4
  s2 <- s
  for (i in 1:7) {
    reg_pile <- s_col==piles2[i]
    if (sum(reg_pile)>0) s2[1:52][reg_pile] <- piles[i]
  }
  if (debug > 0) browser()
  return(s2)
}
