# stdpExecutable: Executable of this repository
# diffInputDirectory: Directory which contains difference of on-center and off-center feedforward weights
# clusterDirectory: Directory which contains cluster files
# outputDirectory: Directory which will contain output images
# totalNeuronNumber: Total neuron number included on the input file
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
    textImageZeroPadding=floor(log10(totalNeuronNumber)+1)
}

do for [cluster=0:n-1] {
    clusterBaseName = sprintf(sprintf('%%0%dd', clusterZeroPadding), cluster);
    clusterFile = sprintf("%s/%s.txt", clusterDirectory, clusterBaseName);
    outputFile = sprintf("%s/%d.svg", outputDirectory, cluster);

    stats clusterFile nooutput
    size=STATS_records

    # TODO: 名前がclustersなのは実態に即していない。imagesやimageNumbersが妥当
    array clusters[size]
    stats clusterFile using (clusters[$0+1]=$1,$0):1 nooutput

    set term svg size 500*10,400*(size/10+1) dynamic
    set output outputFile
    set multiplot title sprintf("Cluster %d", cluster) layout (size/10+1),10
    set size ratio 1

    do for [i=1:size] {
        baseName = sprintf(sprintf('%%0%dd', textImageZeroPadding), clusters[i]);
        inputFile = sprintf("%s/%s.txt", diffInputDirectory, baseName);

        set title sprintf("Neuron %s", baseName)
        set cbrange [-1:1]
        set cbtics (-1, 0, 1)
        set autoscale yfixmin
        set autoscale xfixmin
        set autoscale yfixmax
        set autoscale xfixmax
        unset tics
        set size ratio 1
        set palette defined (0 "#00ff00", 1 "#000000", 2 "#ff00ff")

        plot inputFile matrix with image pixels
        reset
    }
    unset multiplot
}
