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

