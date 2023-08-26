set term svg
set title "Correlation matrix of stimulation"
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
plot "correlation-matrix-stimulation.txt" matrix with image pixels
reset
