# diffInputDirectory: Directory which contains difference of on-center and off-center feedforward weights
# outputFile: File which will contain output image
# n: The number of neurons
# zeroPadding: The number of length of zero padding

if (!exists("zeroPadding")) zeroPadding=0
n=100

set term svg size 500*10,400*(n/10) dynamic
set output "all-wff.svg"

set multiplot layout (n/10+1),10

do for [i=0:n-1] {
    baseName = sprintf(sprintf('%%0%dd', zeroPadding), i);
    diffInputFile = sprintf("%s/%s.txt", "/home/rocktakey/rhq/github.com/wtkd/V1stdp/data/analyze/feedforward-weight-diff", baseName);

    # set title sprintf("Neuron %s", baseName)
    set cbrange [-1:1]
    set cbtics (-1, 0, 1)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    unset tics
    unset colorbox

    set size ratio 1
    # set palette defined (0 "#00ff00", 1 "#000000", 2 "#ff00ff")
    set palette defined (0 "#000000", 1 "#ffffff")

    set border linewidth 5 linecolor "white"

    plot diffInputFile matrix with image pixels notitle
    reset
}

unset multiplot
