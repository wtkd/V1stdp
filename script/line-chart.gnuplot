# inputFile: File which contains data
# outputFile: SVG file name to output.
# title: Title for output graph

if (!exists("title")) title=""

set term svg
set output outputFile
set title title
plot inputFile with linespoints notitle
