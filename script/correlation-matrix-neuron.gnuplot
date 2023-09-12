set term svg
set output "correlation-matrix-neuron.svg"
set title "Correlation matrix of neuron"
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
plot "correlation-matrix-neuron.txt" matrix with image pixels notitle
reset
