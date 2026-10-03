# generate solitaire games
# R.Nachtrieb, 2025-07-27

library(tidyverse)
library(reshape2)

source("get_deck.r")
source("deal.r")
source("load_get.r")

generate_games <- function(debug=0, seeds=seq(0,500),outputcsv = "seed_ch.csv"){
  if (debug > 2) browser()

  nrow <- length(seeds)
  ncol <- 2*52+1
  
  mm <- matrix(data=0,nrow=nrow,ncol=ncol)
  
  for (i in 1:nrow) {
    seed <- seeds[i]
    s <- deal(seed=seed)
    mm[i,1] <- seed
    mm[i,2:ncol] <- s
  }
  df <- as.data.frame(mm)
  
  if (debug > 1) browser()
  write_csv(df,file=outputcsv)
  
  if (debug > 0) browser()
}
