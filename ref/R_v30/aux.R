# XRAY  Description
# 0     Show top card label, replace hidden cards with "-"
# 1     Show all card labels
# 2     Show all card numeric position and face
XRAY <- 0

get_label <- function(x,debug=0){
  if (debug > 2) browser()
  if (length(x) == 0) { 
    return(c())
  } else {
    s <- sign(x)
    v <- abs(x)
    a1 <- ifelse(s==1,"","-")
    a2 <- rep("",length(v))
    reg <- (v >= 1) & (v <= 52)
    n <- sum(reg)
    if (n > 0) {
      a2[reg] <- deck_label[v[reg]]
    }
    l <- paste0(a1,a2)
    return(l)
  }
}


# make a df from a list
get_df_from_ll <- function(ll, debug=0){
  if (debug > 2) browser()
  dd <- matrix(NA, nrow = 24, ncol = 12)
  for (i in 1:12){
    x <- ll[[i]]
    n <- length(x)
    if (n>0) {
      dd[1:n,i] <- x
    }
  }
  df <- as.data.frame(dd)
  names(df) <- names(ll)
  if (debug > 0) browser()
  return(df)
}

# make ll from st
get_ll_from_st <- function(st, debug=0){
  if (debug > 2) browser()
  mm <- st %>% 
    mutate(cardid_sign = card_id*sign) %>% 
    arrange(column,position)
  
  ll <- list("C"=NULL,"D"=NULL,"H"=NULL,"S"=NULL,"P1"=NULL,"P2"=NULL,"P3"=NULL,"P4"=NULL,"P5"=NULL,"P6"=NULL,"P7"=NULL,"ZZ"=NULL)
  for (i in 1:12) {
    v <- NULL
    reg <- mm$column == i
    n <- sum(reg)
    if (n > 0) {
      v <- mm$cardid_sign[reg]
      ll[[i]] <- v
    }
  }
  if (debug > 0) browser()
  return(ll)
}

# make ll from df
get_ll_from_df <- function(df, debug=0){
  if (debug > 2) browser()
  ll <- NULL
  for (i in 1:12) {
    v <- df[[i]] # a vector
    reg <- !is.na(v)
    if (all(!reg)) {
      ll[[i]] <- NULL
    } else {
      ll[[i]] <- df[[i]][reg]
    }
  }
  names(ll) <- c("C","D","H","S","P1","P2","P3","P4","P5","P6","P7","ZZ")
  if (debug > 0) browser()
  return(ll)
}

# make a st from a list
get_st_from_ll <- function(ll, debug=0){
  if (debug > 2) browser()
  st <- NULL
  for (i in 1:12){
    x <- ll[[i]]
    ch <- names(ll)[i]
    n <- length(x)
    if (n>0) {
      card_id <- abs(x)
      column <- i
      position = seq(1,n)
      sign = sign(x)
      this_st <- data.frame(card_id=card_id,column=column,position=position,sign=sign)
      st <- bind_rows(st,this_st)
    }
  }
  st <- st %>%
    arrange(card_id)
  rownames(st) <- NULL
  if (debug > 0) browser()
  return(st)
}

# given a state dataframe, return the integer version
get_s_from_st <- function(st, debug=0){
  if (debug > 2) browser()
  mm <- st %>%
    arrange(card_id) %>%
    mutate(position_sign = position*sign) 
  s <- c(mm$column,mm$position_sign)
  if (debug > 0) browser()
  return(s)
}

# given a state dataframe, return the integer version
get_st_from_s <- function(s, debug=0){
  if (debug > 2) browser()
  n <- length(s)/2
  card_id <- seq(1,n)
  s_col <- s[card_id]
  s_possign <- s[card_id+52]
  s_pos = abs(s_possign)
  s_sign <- sign(s_possign)
  st <- data.frame(card_id=card_id, column=s_col, position=s_pos, sign=s_sign)
  if (debug > 0) browser()
  return(st)
}

# return a df with labels, for viewing only
display_s <- function(s,debug=0){
  if (debug > 2) browser()
  
  st <- get_st_from_s(s, debug=debug)
  ll <- get_ll_from_st(st, debug=debug)    
  df <- get_df_from_ll(ll, debug=debug)
  
  df2 <- df
  for (i in 1:12) {
    v <- df2[[i]] # a vector
    reg <- !is.na(v)
    n <- sum(reg)
    if (n > 0) {
      u <- v[reg]
      x0 <- get_label(pmax(0,u))
      x1 <- get_label(u)
      x2 <- u
      x <- x0 # default
      if (XRAY == 1) {
        x <- x1
      } else if (XRAY == 2) {
        x <- x2
      }
      v2 <- v
      v2[reg] <- x
      df2[i] <- v2
    }
  }
  
  if (debug > 0) browser()
  return(df2)
}

# make a simple dataframe of s and show it
view_s <- function(s, debug=0){
  if (debug > 2) browser()
  st <- get_st_from_s(s)
  ucol <- sort(unique(st$column))
  ncol <- max(st$column)-min(st$column)+1 # assumes consequtive
  nrow <- max(st$position)-min(st$position)+1 # assumes consequtive
  dd <- matrix(NA, nrow = nrow, ncol = ncol)
  for (i in 1:ncol){
    reg <- st$column == ucol[i]
    n <- sum(reg)
    if (n>0) {
      r <- st$position[reg]
      x <- sort(r,index.return=TRUE)
      dd[x$ix,i] <- st$card_id[reg]*st$sign[reg]
    }
  }
  df <- as.data.frame(dd)
  names(df) <- ucol
  print(df)
  if (debug > 0) browser()
}

# matrix display of the state vector
mat_s <- function(s,col=13,row=8,debug=0){
  m <- t(array(s,dim=c(col,row)))
  print(m)
}
