# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.
# title: Title for output graph

if (!exists("title")) title=""

set term svg
set output outputFile
set title title
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
set cbrange [-127:128]
set cbtics (-127, -64, 0, 64, 128)
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
unset tics
set size ratio 1
plot inputFile matrix with image pixels notitle
reset
