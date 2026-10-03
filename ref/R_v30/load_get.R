# convert from s to ch, or vice versa. 
# load ch from .rda

get_ch_from_s <- function(s){
  ch <- paste0(s,collapse=",")
  return(ch)
}

get_s_from_ch <- function(ch){
  s <- as.numeric(unlist(strsplit(ch,",")))
}

load_s <- function(debug=0, rda="gs.rda", n=NULL){
  if (debug > 2) browser()
  load(rda)
  if (is.null(n)) n <- dim(gs)[1]
  ch <- gs$ch[n]
  s <- get_s_from_ch(ch)
  if (debug > 2) browser()
  return(s)
}

