# make a new game. we can set the seed to reproduce results
deal <- function(seed=-1, debug=0){
  if (debug > 2) browser()
  if (!is.null(seed)) set.seed(seed)
  s0_col <- c(5 ,rep(6,2) ,rep(7,3) ,rep(8,4) ,rep(9,5) ,rep(10,6) 
              ,rep(11,7) ,rep(12,24))
  s0_pos <- c(1 ,seq(1,2) ,seq(1,3) ,seq(1,4) ,seq(1,5) ,seq(1,6)  
              ,seq(1,7)  ,seq(1,24))
  s0_face <- c(1 ,c(-1,1), c(rep(-1,2),1), c(rep(-1,3),1), 
               c(rep(-1,4),1), c(rep(-1,5),1), c(rep(-1,6),1), c(rep(c(-1,-1,1),8)))
  n <- 52
  s_col <- rep(NA,n)
  s_possign <- rep(NA,n)
  u <- runif(n)
  if (seed == -1) u <- seq(1,n)/n
  r <- sort(u,index.return=TRUE)
  x <- r$ix
  s_col[x] <- s0_col
  s_possign[x] <- s0_pos*s0_face
  s <- c(s_col, s_possign)
  if (debug > 0) browser()
  return(s)
}