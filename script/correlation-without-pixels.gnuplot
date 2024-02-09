# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.

set term svg size 420,320
set output outputFile

set cbrange [0:1]
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax

set xlabel "刺激画像"
set ylabel "刺激画像"

plot inputFile matrix with image notitle
reset
