# onInputFile: File contains on-center rectangle
# offInputFile: File contains off-center rectangle
# outputFile: SVG file name to output.

set term svg size 300,300
set output outputFile

set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
set size ratio 1
set xrange [0:16]
set yrange [0:16]

set notics

set xlabel "視野X軸"
set ylabel "視野Y軸"

# plot "data/on-rect.txt" with ellipses linecolor "#990099" notitle fs solid 0.1
plot "data/off-rect.txt" with ellipses linecolor "#009900" notitle fs solid 0.1
