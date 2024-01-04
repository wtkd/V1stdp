# inputFile: File which contains data
# outputFile: SVG file name to output.
# title: Title for output graph

if (!exists("title")) title=""

set term svg
set output outputFile
set title title

set style fill solid border linecolor rgb "black"
set xtics 0, 10, 180

binwidth=7.5

set xrange [0:180]

transform(x)=(-x<0?-x+1:-x)*180

plot inputFile using (transform(abs($1-$2) < 0.5 ? ($1+$2)/2 : ($1+$2)/2<0 ? (($1+$2)/2+0.5) : (($1+$2)/2-0.5))) \
     bins binrange [0-binwidth:180-binwidth] binwidth=binwidth with boxes fill solid linecolor rgb "grey" notitle
