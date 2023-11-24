# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.
# title: Title for output graph
# template: Template vector

if (!exists("title")) title=""

set term svg
set output outputFile

set multiplot title title

set size 0.03,0.9
set origin 0.1,0
unset colorbox
set noytics
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
set cbrange [0:40]
plot template using (0):0:1 with lines lc palette lw 20 notitle

reset

set size 0.9,0.9
set origin 0.1,0
set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax
set cbrange [0:40]
plot inputFile matrix with image pixels notitle
