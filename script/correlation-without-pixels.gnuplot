# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.
# title: Title for output graph

if (!exists("title")) title=""

set term svg
set output outputFile
set title title
set cbrange [-1:+1]
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
plot inputFile matrix with image notitle
reset
