set term svg
set output "resps_test_excitatory.svg"
set title "Response of each neuron with each stimulation"
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
plot "resps_test_excitatory.txt" matrix with image pixels notitle
reset