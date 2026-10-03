# solitaire
# R.Nachtrieb, 2025-06-08
# we store the state in two forms.  
# 1. s (integer vector).  Most convenient for manipulation
#   vector of length [1:52,53:104].  First 52 are column of each card
#   second 52 are the postion*sign of each card
# 2. ch (string, comma separated variable).  Most convenient for storage
#   csv of s

library(tidyverse)
library(reshape2)

source("get_deck.r")
source("deal.r")
source("load_get.r")
source("sort_cols.r")
source("get_move.r")
source("make_move.r")
source("one_step.r")


play <- function(debug=0, s=NULL, seed=0){
  if (debug > 2) browser()
  
  if (is.null(s)) s <- deal(seed=seed)
  s <- sort_cols(s)

  gs <- NULL
  keep_going <- TRUE
  i=1
  while(keep_going) {
    this_gs <- data.frame(seed=seed,i=i,ch=get_ch_from_s(s))
    gs <- bind_rows(gs,this_gs)
    s2 <- one_step(s=s,debug=debug,gs=gs)
#    if (debug == -3) browser()
    
    keep_going <- (length(s2) == 104)
    if (keep_going) {
      cat(" suit score",sum(s2[1:52] < 5),"\n")
      s <- s2
      i <- i+1
    } else {
      cat(" FINAL suit score",sum(s[1:52] < 5),"\n")
      if (debug == -3) browser()
      return(gs)
    }
  }
}


sweep <- function(
    debug=0
    , seeds=seq(90,200)
    ,detail_rda = "gs_detail.rda"
    ,summary_rda = "gs_summary.rda"
) {
  gs_detail <- NULL
  gs_summary <- NULL
  
  for (this_seed in seeds){
    this_gs <- play(seed=this_seed)
    
    gs_detail <- bind_rows(gs_detail,this_gs)
    save(gs_detail,file=detail_rda)
    
    n <- dim(this_gs)[1]
    this_seed <- this_gs$seed[n]
    this_ch <- this_gs$ch[n]
    this_s <- get_s_from_ch(this_ch)
    this_score <- sum(this_s[1:52] < 5)
    this_summary <- data.frame(seed=this_seed,score=this_score)
    gs_summary <- bind_rows(gs_summary,this_summary)
    save(gs_summary,file=summary_rda)
    
    make_hist(gs_summary)
  }
}


make_hist <- function(gs_summary){
  n <- dim(gs_summary)[1]
  p <- ggplot(gs_summary,aes(x=score)) + 
    geom_histogram(binwidth=2) + 
    labs(title=paste("N=",n))
  print(p)
}
