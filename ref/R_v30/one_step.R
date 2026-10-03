################################################################
# we start from a single state
# find the moves
# for each of the moves, 
#    make the move, 
#    find the moves
#    for each of the moves, 
#       make the move, 
#       find the moves
#       return the count
################################################################

one_step <- function(debug=0, s=NULL, seed=0, gs=gs){
  if (debug > 2) browser()
  
  if (is.null(s)) s <- deal(seed=seed)
  s0 <- sort_cols(s)
  df <- NULL
  
  # find the moves
  move1 <- get_move(s0, debug=debug)
  N1 <- dim(move1)[1]
  if (N1 > 0) {
    for (i1 in 1:N1) {
      # make the move
      from_id <- move1$from_id[i1]
      to_col <- move1$to_col[i1]
      to_pos <- move1$to_pos[i1]
      s1 <- make_move(s0,from_id,to_col,to_pos,debug=debug)
      s1 <- sort_cols(s1, debug=debug)
      ch1 <- get_ch_from_s(s1)
      
      N2<-0
      N3<-0
      
      # find the moves
      move2 <- get_move(s1, debug=debug)
      N2 <- dim(move2)[1]
      
      if (N2 > 0) {
        for (i2 in 1:N2) {
          
          # make the move
          from_id <- move2$from_id[i2]
          to_col <- move2$to_col[i2]
          to_pos <- move2$to_pos[i2]
          s2 <- make_move(s1,from_id,to_col,to_pos,debug=debug)
          s2 <- sort_cols(s2)
          # find the moves
          move3 <- get_move(s2, debug=debug)
          N3 <- dim(move3)[1]
          
          this_df <- data.frame(
            i1=i1,ch1=ch1,i2=i2,N1=N1,N2=N2,N3=N3
          )
          df <- bind_rows(df,this_df)
          if (debug == -1) browser()
        }
      } 
    }
  } 
  mm0 <- df %>% 
    mutate(N23=N2*N3) %>% 
    filter(!(ch1 %in% gs$ch))
  
  
  if (dim(mm0)[1] > 0) {
    mm <- mm0 %>%
      group_by(i1) %>% 
      summarize(score = mean(N23)) %>% 
      ungroup() %>%
      filter(score == max(score))
    
    this_score <- mm$score[1]
    cat("move score",this_score)
    
    if (this_score > 0) {
      i <- mm$i1[1]
      from_id <- move1$from_id[i]
      to_col <- move1$to_col[i]
      to_pos <- move1$to_pos[i]
      s <- make_move(s0,from_id,to_col,to_pos,debug=debug)
      s <- sort_cols(s)
      if (debug == -2) browser()
      return(s)
    } else {
      return(numeric())
    }
    
  } else {
    return(numeric())
  }
  
}
