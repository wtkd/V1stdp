# onInputFile: File contains on-center rectangle
# offInputFile: File contains off-center rectangle
# outputFile: SVG file name to output.

set term svg
set output outputFile

set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
set size ratio 1
set xrange [0:16]
set yrange [0:16]

set tics 1

plot "data/on-rect.txt" with ellipses linecolor "#990099" notitle, \
     "data/off-rect.txt" with ellipses linecolor "#009900" notitle
