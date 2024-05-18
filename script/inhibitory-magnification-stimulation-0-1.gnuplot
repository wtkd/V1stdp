
# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.
# title: Title for output graph

if (!exists("title")) title=""

set term svg
set output outputFile
set title title

set size ratio 1

set style fill transparent solid 0.05 noborder

set xlabel "抑制性結合強度の倍率"
set ylabel "画像クラスタサイズ(1のものは削除)"
set title "各結合強度倍率におけるクラスタサイズ変化(円の半径はlog(N+1))"

set parametric
set trange [0:100]

plot inputFile using 1:2:($2<=2?0:0.02*sqrt($3+1)) with circles, \
     1,t with line

reset
