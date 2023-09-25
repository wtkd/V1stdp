# onInputDirectory: Directory which contains on-center input text files
# offInputDirectory: Directory which contains off-center input text files
# outputDirectory: Directory which will contain output images
# n: The number of images to output
# zeroPadding: The number of length of zero padding

if (!exists("zeroPadding")) zeroPadding=0

do for [i=0:n-1] {
    baseName = sprintf(sprintf('%%0%dd', zeroPadding), i);
    onInputFile = sprintf("%s/%s.txt", onInputDirectory, baseName);
    offInputFile = sprintf("%s/%s.txt", offInputDirectory, baseName);
    outputFile = sprintf("%s/%s.svg", outputDirectory, baseName);

    set term svg
    set output outputFile

    set multiplot layout 1,2
    set size ratio 1

    set title sprintf("Input on-center image %s", baseName)
    set cbrange [0:128]
    set cbtics (0, 64, 128)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    plot onInputFile matrix with image pixels

    set title sprintf("Input off-center image %s", baseName)
    set cbrange [0:127]
    set cbtics (0, 64, 127)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    plot offInputFile matrix with image pixels

    unset multiplot
}
