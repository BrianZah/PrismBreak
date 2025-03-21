library (MASS)
library (teigen)

main <- function(dim=3, numPoints=1000, save=FALSE) {
  dis = dim+1

  # Create a kx1 matrix of ones
  A <- matrix(1, nrow = dis, ncol = 1)
  qr_decomp <- qr(A) # Perform QR decomposition and extract the complete Q matrix
  Q <- qr.Q(qr_decomp, complete = TRUE)  # Q is a 4x4 orthogonal matrix
  # Exclude the first column of Q and scale the remaining columns by sqrt(2)
  mean <- Q[, 2:ncol(Q)] / sqrt(2)
  print(mean[1,])

  # Create an array of k identity matrices
  cov <- array(rep(diag(dim), dis), dim = c(dim, dim, dis))
  print(cov[,,1])

  data <- numeric(0)
  classification <- numeric(0)
  numPointsLeft <- numPoints
  for(i in 1:dis) {
    numClusterPoints <- ceiling(numPointsLeft/(dis+1-i))
    numPointsLeft <- numPointsLeft-numClusterPoints
    sample <- mvrnorm(numClusterPoints, mean[i,], cov[,,i])
    data <- rbind(data, sample)
    classification <- rbind(classification, matrix(i, nrow = numClusterPoints, ncol = 1))
  }

  if(save) {
    target = paste0('./simplex', '(dim=', dim, '_points=', numPoints, ')')
    dir.create(target)
    write.table(data, paste(target, 'data.csv', sep='/'), sep=",", row.names=TRUE,  col.names=TRUE)

    targetGmm = paste(target, "gmm", sep='/')
    dir.create(targetGmm)
    pig = matrix(1/dis, nrow = dis, ncol = 1)
    write.table(pig,            paste(targetGmm, 'pig.csv', sep='/'),            sep=",", row.names=FALSE, col.names=FALSE)
    df = matrix(Inf, nrow = dis, ncol = 1)
    write.table(df,             paste(targetGmm, 'df.csv', sep='/'),             sep=",", row.names=FALSE, col.names=FALSE)
    write.table(mean,           paste(targetGmm, 'mean.csv', sep='/'),           sep=",", row.names=FALSE, col.names=FALSE)
    d = dim(cov)
    sigma <- array(aperm(cov, perm=c(1,3,2)), dim=c(d[1]*d[3], d[2]))
    write.table(sigma,          paste(targetGmm, 'sigma.csv', sep='/'),          sep=",", row.names=FALSE, col.names=FALSE)
    write.table(data,           paste(targetGmm, 'scaled.csv', sep='/'),         sep=",", row.names=TRUE,  col.names=TRUE)
    write.table(classification, paste(targetGmm, 'classification.csv', sep='/'), sep=",", row.names=FALSE, col.names=FALSE)

    targetTmm = paste(target, "tmm", sep='/')
    dir.create(targetTmm)
    pig = matrix(1/dis, nrow = dis, ncol = 1)
    write.table(pig,            paste(targetTmm, 'pig.csv', sep='/'),            sep=",", row.names=FALSE, col.names=FALSE)
    df = matrix(100, nrow = dis, ncol = 1)
    write.table(df,             paste(targetTmm, 'df.csv', sep='/'),             sep=",", row.names=FALSE, col.names=FALSE)
    write.table(mean,           paste(targetTmm, 'mean.csv', sep='/'),           sep=",", row.names=FALSE, col.names=FALSE)
    d = dim(cov)
    sigma <- array(aperm(cov, perm=c(1,3,2)), dim=c(d[1]*d[3], d[2]))
    write.table(sigma,          paste(targetTmm, 'sigma.csv', sep='/'),          sep=",", row.names=FALSE, col.names=FALSE)
    write.table(data,           paste(targetTmm, 'scaled.csv', sep='/'),         sep=",", row.names=TRUE,  col.names=TRUE)
    write.table(classification, paste(targetTmm, 'classification.csv', sep='/'), sep=",", row.names=FALSE, col.names=FALSE)

  }
}

main(dim=24, numPoints=10000, save=TRUE)
