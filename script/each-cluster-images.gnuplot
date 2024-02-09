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

stdpExecutable="/home/rocktakey/rhq/github.com/wtkd/V1stdp/build/src/main/stdp"
textImageDirectory="/home/rocktakey/rhq/github.com/wtkd/V1stdp/out/images/imagesText"
clusterDirectory="/home/rocktakey/rhq/github.com/wtkd/V1stdp/script/clusterMapStimulation"
outputDirectory="stimulation"
totalImageNumber=109999
imageRange=1000
n=13
clusterZeroPadding=0
textImageZeroPadding=6

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

    # TODO: 名前がclustersなのは実態に即していない。imagesやimageNumbersが妥当
    array clusters[size]
    stats clusterFile using (clusters[$0+1]=$1,$0):1 nooutput

    set term svg size 500*10,400*(size/10+1) dynamic
    set output outputFile
    set multiplot layout (size/10+1),10
    set size ratio 1

    do for [i=1:size] {
        imageNumberFromTop=clusters[i]
        # FIXME: ワークアラウンドでこう書いているが、実際はこちらではなくプログラム本体を直すべきである
        # imageNumberFromTop + imageTop

        baseName = sprintf(sprintf('%%0%dd', textImageZeroPadding), (ceil(imageNumberFromTop) % imageRange) + (imageTop + 1));
        inputFile = sprintf("%s/%s.txt", textImageDirectory, baseName);

        set cbrange [-127:128]
        set cbtics (-127, -64, 0, 64, 128)
        set autoscale yfixmin
        set autoscale xfixmin
        set autoscale yfixmax
        set autoscale xfixmax
        unset tics
        set size ratio 1

        set palette defined (0 "#000000", 1 "#ffffff")

        unset colorbox
        set border linewidth 3 linecolor "white"

        plot inputFile matrix with image pixels
        reset
    }
    unset multiplot
}
