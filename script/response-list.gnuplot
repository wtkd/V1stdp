# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.
# n: The number of neurons
# title: Title for output graph

if (!exists("title")) title=""

set term png size 500*10,400*(n/10)
set output outputFile
set title title
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax

set multiplot title sprintf("Tuning curves") layout (n/10+1),10

do for [i=0:n-1] {
    set xlabel "Neuron"
    set ylabel "Response"
    plot inputFile matrix every :::i::i using 1:3 with filledcurves x1 notitle
    reset
}

unset multiplot
