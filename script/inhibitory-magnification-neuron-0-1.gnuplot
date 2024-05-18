
# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.
# title: Title for output graph

if (!exists("title")) title=""

set term svg
set output outputFile
set title title

set size ratio 1

set style fill transparent solid 0.1 noborder

set xlabel "抑制性結合強度の倍率"
set ylabel "神経細胞クラスタサイズ"

set parametric
set trange [0:12]

plot inputFile using 1:2:(0.03*sqrt($3)) with circles, \
     1,t with line


reset
