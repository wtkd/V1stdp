# inputDirectory
# outputFile
# totalIterationNumber
# title
# interval

if(!exists("interval")) {
    interval=10
}

set term svg size 500*10,400*(totalIterationNumber/10/interval+1)
set output sprintf("%s",outputFile)
set multiplot title title layout (totalIterationNumber/10/interval+1),10
set size ratio 1

do for [i=0:totalIterationNumber/interval]{
    n=i*interval
    set title sprintf("%d", n)
    set cbrange [-127:128]
    set cbtics (-127, -64, 0, 64, 128)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax

    plot sprintf("%s/%d.txt",inputDirectory, n) matrix with image
}

unset multiplot
