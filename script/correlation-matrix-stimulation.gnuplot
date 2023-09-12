set term svg
set output "correlation-matrix-stimulation.svg"
set title "Correlation matrix of stimulation"
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax

# Do not use "pixel" because the number of stimulations is too large
plot "correlation-matrix-stimulation.txt" matrix with image notitle
reset
