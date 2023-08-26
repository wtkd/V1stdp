set term svg
set title "Response of each neuron with each stimulation"
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
plot "resps_test_excitatory_sorted.txt" matrix with image pixels
reset
