# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.

set term svg size 420,320
set output outputFile
set cbrange [0:1]
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax

set xlabel "抑制性神経細胞"
set ylabel "抑制性神経細胞"

plot inputFile matrix with image pixels notitle
reset
