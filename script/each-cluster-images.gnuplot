# stdpExecutable: Executable of this repository
# textImageDirectory: Directory which contains input text images
# clusterDirectory: Directory which contains cluster files
# outputDirectory: Directory which will contain output images
# totalImageNumber: Total image number included on the input file
# imageRange: Indicate range of images used on test. See --image-range option of "stdp test" command.
# numberFile: File which contains the number of clusters
# n: The number of clusters
# clusterZeroPadding: The number of length of zero padding on cluster file
# textImageZeroPadding: The number of length of zero padding used on file names of text images

system sprintf("%s tool filesystem make-directory %s", stdpExecutable, outputDirectory)

if(!exists("n")) {
    stats numberFile using (n=$1,$0) nooutput
}

if(n == 0) {
    exit
}

if(!exists("clusterZeroPadding")) {
    clusterZeroPadding=floor(log10(n)+1+0.0001)
}

if(!exists("textImageZeroPadding")) {
    textImageZeroPadding=floor(log10(totalImageNumber)+1)
}

if (0 < imageRange){
    imageTop = totalImageNumber - imageRange
} else {
    imageTop = 0
}

do for [cluster=0:n-1] {
    clusterBaseName = sprintf(sprintf('%%0%dd', clusterZeroPadding), cluster);
    clusterFile = sprintf("%s/%s.txt", clusterDirectory, clusterBaseName);
    outputFile = sprintf("%s/%d.svg", outputDirectory, cluster);

    stats clusterFile nooutput
    size=STATS_records

    array clusters[size]
    stats clusterFile using (clusters[$0+1]=$1,$0):1 nooutput

    set term svg size 500*(size/5+1),400*5 dynamic
    set output outputFile
    set multiplot title sprintf("Cluster %d", cluster) layout 5,(size/5+1)
    set size ratio 1

    do for [i=1:size] {
        imageNumberFromTop=clusters[i]
        baseName = sprintf(sprintf('%%0%dd', textImageZeroPadding), imageNumberFromTop + imageTop);
        inputFile = sprintf("%s/%s.txt", textImageDirectory, baseName);

        set title sprintf("Input image %s", baseName)
        set cbrange [-127:128]
        set cbtics (-127, -64, 0, 64, 128)
        set autoscale yfixmin
        set autoscale xfixmin
        set autoscale yfixmax
        set autoscale xfixmax
        unset tics
        set size ratio 1

        plot inputFile matrix with image pixels
        reset
    }
    unset multiplot
}
