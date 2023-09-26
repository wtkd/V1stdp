(define learn
  (cwl-workflow "learn.cwl"))

(define test
  (cwl-workflow "test.cwl"))

(define analyze
  (cwl-workflow "analyze.cwl"))

(define export-images
  (cwl-workflow "exportImages.cwl"))

(workflow ((stdp-executable #:type File
                            #:default '((class . "File")
                                        (location . "../build/src/main/stdp")))
           (seed #:type int #:default 0)
           (step-learn #:type int #:default 1000000)
           (step-test #:type int #:default 1000)
           (excitatory-neuron-number #:type int #:default 100)
           (inhibitory-neuron-number #:type int #:default 20)

           ;; For Analyze
           (correlation-threshold-neuron #:type float #:default 0.9)
           (minimum-cluster-size-neuron #:type int #:default 10)
           (correlation-threshold-stimulation #:type float #:default 0.9)
           (minimum-cluster-size-stimulation #:type int #:default 10)

           (correlation-plot-script
            #:type File
            #:default '((class . "File")
                        (location . "../script/correlation.gnuplot")))
           (correlation-plot-script-without-pixels
            #:type File
            #:default '((class . "File")
                        (location . "../script/correlation-without-pixels.gnuplot")))
           (matrix-plot-script
            #:type File
            #:default '((class . "File")
                        (location . "../script/matrix.gnuplot")))
           (each-cluster-images-script #:type File
                                       #:default '((class . "File")
                                                   (location . "../script/each-cluster-images.gnuplot")))

           (output-weight-sorted-txt #:type string #:default "weight-sorted.txt")

           (output-weight-sorted-svg #:type string #:default "weight-sorted.svg")
           (title-weight-sorted #:type string #:default "Weight between each neuron")

           (output-correlation-matrix-neuron #:type string #:default "correlation-matrix-neuron.svg")
           (title-correlation-matrix-neuron #:type string #:default "Correlation matrix of response of each neuron")

           (output-correlation-matrix-stimulation #:type string #:default "correlation-matrix-stimulation.svg")
           (title-correlation-matrix-stimulation #:type string #:default "Correlation matrix of response of each stimulation")

           (output-cluster-map-neuron #:type string #:default "cluster-map-neuron.txt")
           (output-directory-cluster-map-neuron #:type string #:default "cluster-map-neuron")
           (output-number-cluster-map-neuron #:type string #:default "cluster-map-neuron-number.txt")

           (output-cluster-map-stimulation #:type string #:default "cluster-map-stimulation.txt")
           (output-directory-cluster-map-stimulation #:type string #:default "cluster-map-stimulation")
           (output-number-cluster-map-stimulation #:type string #:default "cluster-map-stimulation.txt")

           (output-cluster-images-directory #:type string #:default "clusterImages")

           ;; For export-images
           (transformed-image-data #:type File
                                   #:default '((class . "File")
                                               (location . "../patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat")))
           (image-number #:type int #:default 109999)
           (edge-length #:type int #:default 17)

           (on-image-directory-name #:type string
                                    #:default "onImagesText")
           (off-image-directory-name #:type string
                                     #:default "offImagesText")
           (text-image-directory-name #:type string
                                      #:default "imagesText")
           (svg-image-directory-name #:type string
                                     #:default "svgInputImages")

           (generate-svg-images-script #:type File
                                       #:default '((class . "File")
                                                   (location . "../script/onOffImages.gnuplot")))

           (output-generate-svg-images #:type string
                                       #:default "inputImages"))
          (pipe (tee
                 (learn #:stdp-executable stdp-executable
                       #:transformed-image-data transformed-image-data
                       #:step step-learn
                       #:seed seed)
                 (export-images
                  #:stdp-executable stdp-executable
                  #:transformed-image-data transformed-image-data
                  #:image-number image-number
                  #:edge-length edge-length

                  #:on-image-directory-name on-image-directory-name
                  #:off-image-directory-name off-image-directory-name
                  #:text-image-directory-name text-image-directory-name
                  #:svg-image-directory-name svg-image-directory-name
                  #:generate-svg-images-script generate-svg-images-script
                  #:output-generate-svg-images output-generate-svg-images))
                (tee
                 (rename #:lateral-weight lateral-weight-txt
                         #:feedforward-weight feedforward-weight-txt)
                 (test #:stdp-executable stdp-executable
                       #:transformed-image-data transformed-image-data
                       #:step step-learn
                       #:seed seed
                       #:lateral-weight-dat lateral-weight-dat
                       #:feedforward-weight-dat feedforward-weight-dat))
                (analyze
                 #:stdp-executable stdp-executable
                 #:response-test resps-test
                 #:lateral-weight lateral-weight
                 #:text-image-directory text-images-directory
                 #:excitatory-neuron-number excitatory-neuron-number
                 #:inhibitory-neuron-number inhibitory-neuron-number
                 #:test-stimulation-number step-test
                 #:correlation-threshold-neuron correlation-threshold-neuron
                 #:minimum-cluster-size-neuron minimum-cluster-size-neuron
                 #:correlation-threshold-stimulation correlation-threshold-stimulation
                 #:minimum-cluster-size-stimulation minimum-cluster-size-stimulation
                 #:images-number image-number
                 #:correlation-plot-script correlation-plot-script
                 #:correlation-plot-script-without-pixels correlation-plot-script-without-pixels
                 #:matrix-plot-script matrix-plot-script
                 #:each-cluster-images-script each-cluster-images-script
                 #:output-weight-sorted-txt output-weight-sorted-txt
                 #:output-weight-sorted-svg output-weight-sorted-svg
                 #:title-weight-sorted title-weight-sorted
                 #:output-correlation-matrix-neuron output-correlation-matrix-neuron
                 #:title-correlation-matrix-neuron title-correlation-matrix-neuron
                 #:output-correlation-matrix-stimulation output-correlation-matrix-stimulation
                 #:title-correlation-matrix-stimulation title-correlation-matrix-stimulation
                 #:output-cluster-map-neuron output-cluster-map-neuron
                 #:output-directory-cluster-map-neuron output-directory-cluster-map-neuron
                 #:output-number-cluster-map-neuron output-number-cluster-map-neuron
                 #:output-cluster-map-stimulation output-cluster-map-stimulation
                 #:output-directory-cluster-map-stimulation output-directory-cluster-map-stimulation
                 #:output-number-cluster-map-stimulation output-number-cluster-map-stimulation
                 #:output-cluster-images-directory output-cluster-images-directory)))
