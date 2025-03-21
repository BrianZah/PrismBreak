library (MASS)
library (teigen)

reorder <- function(len) {
  index_reordered <- integer(len)
  print("insert distribution order:")
  for(i in 1:len) {
      print(sprintf("%i = ", i))
      var = readLines("stdin",n=1);
      var = as.integer(var)
      while(var < 1 || len < var || is.element(var, index_reordered)) {
        print("invalid index, insert distribution index again")
        print(sprintf("%i = ", i))
        var = readLines("stdin",n=1);
        var = as.integer(var)
      }
      index_reordered[i] = var
    }
  return(index_reordered)
}

saveMixtureModel <- function(model, name, rowNames, target) {
  dir.create(target)
  print(sprintf("Do you wish to reorder %s distributions?[T/F]", name))
  var = readLines("stdin",n=1);
  var = as.logical(var)
  if(var) {
    d = dim(model$parameters$sigma)
    index_reordered = reorder(d[3])
    pig = model$parameters$pig[index_reordered]
    df = model$parameters$df[index_reordered]
    mean = model$parameters$mean[index_reordered, ]
    sigma = model$parameters$sigma[, , index_reordered]

    classification <- index_reordered[model$classification]

    print(model$parameters$pig)
    print(pig)
    print(model$parameters$df)
    print(df)
    print(model$parameters$mean)
    print(mean)
    print(model$parameters$sigma)
    print(sigma)

    sigma <- array(aperm(sigma, perm=c(1,3,2)), dim=c(d[1]*d[3], d[2]))

    write.table(pig,                      paste(target, 'pig.csv', sep='/'),            sep=",", row.names=FALSE, col.names=FALSE)
    write.table(df,                       paste(target, 'df.csv', sep='/'),             sep=",", row.names=FALSE, col.names=FALSE)
    write.table(mean,                     paste(target, 'mean.csv', sep='/'),           sep=",", row.names=FALSE, col.names=FALSE)
    write.table(sigma,                    paste(target, 'sigma.csv', sep='/'),          sep=",", row.names=FALSE, col.names=FALSE)
    write.table(cbind(rowNames, model$x), paste(target, 'scaled.csv', sep='/'),         sep=",", row.names=FALSE, col.names=TRUE)
    write.table(classification,           paste(target, 'classification.csv', sep='/'), sep=",", row.names=FALSE, col.names=FALSE)
  } else {
    d = dim(model$parameters$sigma)
    sigma <- array(aperm(model$parameters$sigma, perm=c(1,3,2)), dim=c(d[1]*d[3], d[2]))

    write.table(model$parameters$pig,     paste(target, 'pig.csv', sep='/'),            sep=",", row.names=FALSE, col.names=FALSE)
    write.table(model$parameters$df,      paste(target, 'df.csv', sep='/'),             sep=",", row.names=FALSE, col.names=FALSE)
    write.table(model$parameters$mean,    paste(target, 'mean.csv', sep='/'),           sep=",", row.names=FALSE, col.names=FALSE)
    write.table(sigma,                    paste(target, 'sigma.csv', sep='/'),          sep=",", row.names=FALSE, col.names=FALSE)
    write.table(cbind(rowNames, model$x), paste(target, 'scaled.csv', sep='/'),         sep=",", row.names=FALSE, col.names=TRUE)
    write.table(model$classification,     paste(target, 'classification.csv', sep='/'), sep=",", row.names=FALSE, col.names=FALSE)
  }
}

main <- function() {
  args = commandArgs(trailingOnly=TRUE)
  if(length(args) < 2 || length(args) > 3) {
    stop("[Error] usage: Rscript Clustering.r <filename[path/to/csv]> <save[T/F]> <optional: label[path/to/csv]>", call.=FALSE)
  }

  filename = args[1]
  save = as.logical(args[2])
  labelfile <- if(length(args) == 2) "" else args[3]
  print(sprintf("[Info] Read parameters: filename=%s save=%s labelfile=%s", filename, save, labelfile))
  if(is.na(save)) {
    stop("[Error] usage: Rscript Clustering.r <filename[path/to/csv]> <save[T/F]> <optional: label[path/to/csv]>", call.=FALSE)
  }
  df = read.csv(filename)
  labels <- if(labelfile == "") NULL else as.vector(read.csv(labelfile)[, 1])
  print(labels)
  target = substr(filename, 1, nchar(filename)-4)
  print(target)

  print(length(labels))
  print(nrow(df))
  #return()
  #df2 <- df
  #df2 <- scale(df)
  #print(df2)
  #print(colMax(df2))
  #print(colMin(df2))
  rowNames = df[1]
  df = df[-1]

  g <- teigen(df, Gs=1:20, models = "gaussian", scale = TRUE, known = labels)
  t <- teigen(df, Gs=1:20, models = "all", scale = TRUE, known = labels)
  plot(t, what = "contour")
  print(summary(t))
  print(g$parameters$pig)
  print(t$parameters$pig)
  print(g$parameters$df)
  print(t$parameters$df)
  print(g$parameters$mean)
  print(t$parameters$mean)
  print(g$parameters$sigma)
  print(t$parameters$sigma)
  print(t$classification)
  print(t$x)

  if(save) {
    dir.create(target)
    write.table(df, paste(target, 'data.csv', sep='/'), sep=",", row.names=TRUE,  col.names=TRUE)
    saveMixtureModel(g, "guassian", rowNames, paste(target, "gmm", sep='/'))
    saveMixtureModel(t, "t", rowNames, paste(target, "tmm", sep='/'))
  }
}

main()
