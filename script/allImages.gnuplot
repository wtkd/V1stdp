# inputDirectory: Directory which contains input text files
# outputDirectory: Directory which will contain output images
# n: The number of images to output
# zeroPadding: The number of length of zero padding

if (!exists("zeroPadding")) zeroPadding=0

do for [i=1:n] {
    baseName = sprintf(sprintf('%%0%dd', zeroPadding), i);
    inputFile = sprintf("%s/%s.txt", inputDirectory, baseName);
    outputFile = sprintf("%s/%s.svg", outputDirectory, baseName);

    print inputFile

    set term svg
    set output outputFile
    set title sprintf("Input image %s", baseName)
    set cbrange [-127:128]
    set cbtics (-127, -64, 0, 64, 128)
    set autoscale yfixmin
    set autoscale xfixmin
    set autoscale yfixmax
    set autoscale xfixmax
    plot inputFile matrix with image pixels
    reset

}
