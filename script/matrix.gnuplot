# inputFile: File which contains correlation matrix.
# outputFile: SVG file name to output.

set term svg size 420,320
set output "weight-sorted.svg"

set autoscale yfixmin
set autoscale xfixmin
set autoscale yfixmax
set autoscale xfixmax

# set xlabel "刺激画像"
# set ylabel "興奮性神経細胞"

set xlabel "シナプス前細胞" textcolor "white"
set ylabel "シナプス後細胞" textcolor "white"

# set cbrange [50:100]
# set xlabel "刺激画像"
# set ylabel "抑制性神経細胞"

set border linecolor "white"

plot "/home/rocktakey/rhq/github.com/wtkd/V1stdp/data/weight-sorted.txt" matrix with image pixels notitle
reset
