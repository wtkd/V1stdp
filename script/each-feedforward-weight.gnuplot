# onInputDirectory: Directory which contains on-center feedforward weights
# offInputDirectory: Directory which contains off-center feedforward weights
# diffInputDirectory: Directory which contains difference of on-center and off-center feedforward weights
# outputDirectory: Directory which will contain output images
# n: The number of neurons
# zeroPadding: The number of length of zero padding
# stdpExecutable: Executable of this repository

if (!exists("zeroPadding")) zeroPadding=0

system sprintf("%s tool filesystem make-directory %s", stdpExecutable, outputDirectory)

do for [i=0:n-1] {
    baseName = sprintf(sprintf('%%0%dd', zeroPadding), i);
    onInputFile = sprintf("%s/%s.txt", onInputDirectory, baseName);
    offInputFile = sprintf("%s/%s.txt", offInputDirectory, baseName);
    diffInputFile = sprintf("%s/%s.txt", diffInputDirectory, baseName);
    outputFile = sprintf("%s/%s.svg", outputDirectory, baseName);

    set term svg size 500*3,400 dynamic
    set output outputFile

    set multiplot layout 1,3
    set size ratio 1

    set title sprintf("Feedforward weights for on-center of neuron %s", baseName)
    set cbrange [0:1]
    set cbtics (0, 1)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    plot onInputFile matrix with image pixels notitle
    reset

    set title sprintf("Feedforward weights for off-center of neuron %s", baseName)
    set cbrange [0:1]
    set cbtics (0, 1)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    plot offInputFile matrix with image pixels notitle
    reset

    set title sprintf("Difference of feedforward weights for on-center and off-center of neuron %s", baseName)
    set cbrange [-1:1]
    set cbtics (-1, 0, 1)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    set palette defined (0 "#00ff00", 1 "#000000", 2 "#ff00ff")
    plot diffInputFile matrix with image pixels notitle
    reset

    unset multiplot
}
