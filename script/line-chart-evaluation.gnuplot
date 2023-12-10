# inputFile: File which contains data
# outputFile: SVG file name to output.
# title: Title for output graph

if (!exists("title")) title=""

set term svg
set output outputFile
set title title

set key autotitle columnhead
set key left top

plot inputFile using 2 with linespoints lw 5, \
     "" using 3 with linespoints, \
     "" using 4 with linespoints, \
     "" using 5 with linespoints, \
     "" using 6 with linespoints, \
     "" using 7 with linespoints
