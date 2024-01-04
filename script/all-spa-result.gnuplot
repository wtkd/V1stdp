# inputDirectory: Directory which contains text image
# outputFile: File which will contain output image
# n: The number of neurons
# zeroPadding: The number of length of zero padding

if (!exists("zeroPadding")) zeroPadding=0

set term svg size 500*10,400*(n/10) dynamic
set output outputFile

set multiplot title sprintf("SPA result") layout (n/10+1),10

do for [i=0:n-1] {
    baseName = sprintf(sprintf('%%0%dd', zeroPadding), i);
    diffInputFile = sprintf("%s/%s.txt", inputDirectory, baseName);

    set title sprintf("Neuron %s", baseName)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    set size ratio 1

    stats diffInputFile matrix nooutput
    M=abs(STATS_max)>abs(STATS_min)?abs(STATS_max):abs(STATS_min)
    set cbrange [-M:M]

    plot diffInputFile matrix with image pixels notitle
    reset
}

unset multiplot
