main <- function() {
  df = read.csv("../data/shanghai_ranking_2024.csv")
  target = "../data/shanghai_ranking_2024_processed.csv"
  target
  df <- df[c("University_Name","Alumni","Award","Hici","N.S","PUB","PCP")]
  df <- df[complete.cases(df), ]
  df
  write.csv(df, target, row.names=FALSE)
}

main()
