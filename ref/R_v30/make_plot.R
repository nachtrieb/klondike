library(tidyverse)

agg_gs <- function(
    debug=0
    ,all_summary_rda = c(
      "gs_summary_0_89.rda"
      ,"gs_summary_90_200.rda"
      ,"gs_summary_201_221.rda"
      ,"gs_summary_223_300.rda"
      ,"gs_summary_301_316.rda"
      ,"gs_summary_318_400.rda"
      ,"gs_summary_401_500.rda"
    )
    ,all_detail_rda = c(
      "gs_detail_0_89.rda"
      ,"gs_detail_90_200.rda"
      ,"gs_detail_201_221.rda"
      ,"gs_detail_223_300.rda"
      ,"gs_detail_301_316.rda"
      ,"gs_detail_318_400.rda"
      ,"gs_detail_401_500.rda"
    )
){
  if (debug > 2) browser()
  
  all_gs_detail <- NULL
  for (this_rda in all_detail_rda) {
    load(this_rda)
    all_gs_detail <- bind_rows(all_gs_detail,gs_detail)
  }
  save(all_gs_detail,file="gs_detail.rda")
  
  all_gs_summary <- NULL
  for (this_rda in all_summary_rda) {
    load(this_rda)
    all_gs_summary <- bind_rows(all_gs_summary,gs_summary)
  }
  save(all_gs_summary,file="gs_summary.rda")
  
  return(all_gs_summary)
}


make_plot <- function(
    gs_summary
    ,plot=FALSE
    ,debug=0
    ,binwidth=2
){
  if(debug > 2) browser()
  
  xmax <- 40

  mm1 <- gs_summary %>%
    filter(score <= xmax) %>%
    summarize(
      games = n()
      ,tot = sum(score)
      ,mX = mean(score)
      ,vX = var(score)
    ) 
  
  xx <- seq(0*binwidth/2,xmax,binwidth)
  
  mX <- mm1$mX
  vX <- mm1$vX
  sX <- sqrt(vX)
  
  ml <- log(mX*mX/sqrt(mX*mX+vX))
  vl <- log(1+vX/(mX*mX))
  sl <- sqrt(vl)
  
  
  nsum <- mm1$games
  #yy <- dpois(xx,mu)*ntot*binwidth
  yy <- dlnorm(xx,meanlog=ml,sdlog=sl)*nsum*binwidth
  mm <- data.frame(x=xx,y=yy) 
  
  
  
  
  ntot <- dim(gs_summary)[1]
  
  p <- ggplot(gs_summary,aes(x=score)) + 
    geom_histogram(binwidth=binwidth,fill='#69b2a2',color='#e9ecef' ) + 
    labs(x="Final Score",y="Number of Games"
         ,title=paste0("Total Games = ",ntot,", Winners = ",ntot-nsum," (",round(100*(1-nsum/ntot),0),"%)")) + 
    geom_line(data=mm,aes(x=x,y=y), color="red")
  
  
  
  if (plot) pdf(file="gs_summary.pdf",width=7,height=5)
  print(p)
  if (plot) dev.off()
  
  if(debug > 0) browser()
}
