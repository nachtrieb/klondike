# generate solitaire games
# R.Nachtrieb, 2025-07-27

library(tidyverse)
library(reshape2)

source("get_deck.r")
source("deal.r")
source("load_get.r")


generate_games <- function(debug=0, seeds=seq(0,500),games_rda = "seed_ch.rda"){
  if (debug > 2) browser()

  df <- NULL
  for (i in seeds) {
    s <- deal(seed=i)
    ch <- get_ch_from_s(s)
    this_df <- data.frame(seed=i,ch=ch)
    df <- bind_rows(df,this_df)
  }
    
  if (debug > 1) browser()
  save(df,file=games_rda)
  
  if (debug > 0) browser()
}
