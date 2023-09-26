(define response-cut-out-excitatory
  (command #:inputs
           (stdp-executable #:type File)
           (response #:type File)
           (excitatory-neuron-number #:type int)
           (inhibitory-neuron-number #:type int)
           (stimulation-number #:type int)
           #:run
           stdp-executable "tool" "analyze" "response" "cut"
           response
           "--excitatory-neuron-number" excitatory-neuron-number
           "--inhibitory-neuron-number" inhibitory-neuron-number
           "--stimulation-number" stimulation-number
           "--excitatory-only-output" "response-excitatory.txt"
           #:outputs
           (response-excitatory #:type File
                                #:binding '((glob . "response-excitatory.txt")))))

(define response-clustering
  (command #:inputs
           (stdp-executable #:type File)
           (response #:type File)
           (neuron-number #:type int)
           (stimulation-number #:type int)
           #:run
           stdp-executable "tool" "analyze" "response" "clustering"
           response "response-sorted.txt"
           "--neuron-number" neuron-number
           "--stimulation-number" stimulation-number
           "--neuron" "sort-index-neuron.txt"
           "--stimulation" "sort-index-stimulation.txt"
           #:outputs
           (response-sorted #:type File
                            #:binding '((glob . "response-sorted.txt")))
           (sort-index-neuron #:type File
                              #:binding '((glob . "sort-index-neuron.txt")))
           (sort-index-stimulation #:type File
                                   #:binding '((glob . "sort-index-stimulation.txt")))))

(define lateral-weight-cut-out-excitatory
  (command #:inputs
           (stdp-executable #:type File)
           (lateral-weight #:type File)
           (excitatory-neuron-number #:type int)
           (inhibitory-neuron-number #:type int)
           #:run
           stdp-executable "tool" "analyze" "weight" "cut"
           lateral-weight
           "--excitatory-neuron-number" excitatory-neuron-number
           "--inhibitory-neuron-number" inhibitory-neuron-number
           "--excitatory-only-output" "weight-excitatory.txt"
           #:outputs
           (weight-excitatory #:type File
                              #:binding '((glob . "weight-excitatory.txt")))))

(define sort-lateral-weight
  (command #:inputs
           (stdp-executable #:type File)
           (weight-excitatory #:type File)
           (sort-index-neuron-row #:type File)
           (sort-index-neuron-colomn #:type File)
           (output-name #:type string)
           #:run
           stdp-executable "tool" "analyze" "apply-permutation"
           weight-excitatory output-name
           "--colomn" sort-index-neuron-row
           "--row" sort-index-neuron-colomn
           #:outputs
           (weight-sorted #:type File
                          #:binding '((glob . "$(inputs[\"output-name\"])")))))

(define response-correlation-matrix
  (command #:inputs
           (stdp-executable #:type File)
           (response #:type File)
           (neuron-number #:type int)
           (stimulation-number #:type int)
           #:run
           stdp-executable "tool" "analyze" "response" "correlation-matrix"
           response
           "--neuron-number" neuron-number
           "--stimulation-number" stimulation-number
           "--neuron" "correlation-matrix-neuron.txt"
           "--stimulation" "correlation-matrix-stimulation.txt"
           #:outputs
           (correlation-matrix-neuron #:type File
                                      #:binding '((glob . "correlation-matrix-neuron.txt")))
           (correlation-matrix-stimulation #:type File
                                           #:binding '((glob . "correlation-matrix-stimulation.txt")))))

(define cluster-map
  (command #:inputs
           (stdp-executable #:type File)
           (correlation-matrix #:type File)
           (input-size #:type int)
           (correlation-threshold #:type float)
           (minimum-cluster-size #:type int)
           (output-name #:type string)
           #:run
           stdp-executable "tool" "analyze" "response" "cluster-map"
           response correlation-matrix output-name
           "--input-size" input-size
           "--correlation-threshold" correlation-threshold
           "--minimum-cluster-size" minimum-cluster-size
           #:outputs
           (cluster-map #:type File
                        #:binding '((glob . "$(inputs[\"output-name\"])")))))

(define divide-line
  (command #:inputs
           (stdp-executable #:type File)
           (cluster-map #:type File)
           (output-directory #:type string)
           (output-number-file #:type string)
           #:run
           stdp-executable "tool" "analyze" "divide-line"
           cluster-map output-directory
           "--number-output" output-number-file
           #:outputs
           (divided-directory #:type Directory
                              #:binding '((glob . "$(inputs[\"output-directory\"])")))
           (number-file #:type File
                        #:binding '((glob . "$(inputs[\"output-number-file\"])")))))

(define plot-correlation-neuron
  (command #:inputs
           (gnuplot-script #:type File)
           (correlation-matrix #:type File)
           (output-name #:type string)
           (title #:type string)
           #:run
           "gnuplot"
           "-e" "inputFile='$(inputs[\"correlation-matrix\"].path)'"
           "-e" "outputFile='$(inputs[\"output-name\"])'"
           "-e" "title='$(inputs.title)'"
           gnuplot-script
           #:outputs
           (correlation-plot-neuron #:type File
                                    #:binding '((glob . "$(inputs[\"output-name\"])")))))

(define plot-correlation-stimulation
  (command #:inputs
           (gnuplot-script #:type File)
           (correlation-matrix #:type File)
           (output-name #:type string)
           (title #:type string)
           #:run
           "gnuplot"
           "-e" "inputFile='$(inputs[\"correlation-matrix\"].path)'"
           "-e" "outputFile='$(inputs[\"output-name\"])'"
           "-e" "title='$(inputs.title)'"
           gnuplot-script
           #:outputs
           (correlation-plot-plot-correlation-stimulation #:type File
                                                          #:binding '((glob . "$(inputs[\"output-name\"])")))))

(define plot-matrix
  (command #:inputs
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/correlation.gnuplot")))
           (matrix #:type File)
           (output-name #:type string)
           (title #:type string)
           #:run
           "gnuplot"
           "-e" "inputFile='$(inputs.matrix.path)'"
           "-e" "outputFile='$(inputs[\"output-name\"])'"
           "-e" "title='$(inputs.title)'"
           gnuplot-script
           #:outputs
           (matrix-plot #:type File
                        #:binding '((glob . "$(inputs[\"output-name\"])")))))

(define each-cluster-images
  (command #:inputs
           (stdp-executable #:type File)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/each-cluster-images.gnuplot")))
           (text-image-directory #:type Directory)
           (cluster-directory #:type Directory)
           (number-file #:type File)
           (images-number #:type int)
           (output-directory #:type string)
           #:run
           "gnuplot"
           "-e" "textImageDirectory='$(inputs[\"text-image-directory\"].path)'"
           "-e" "clusterDirectory='$(inputs[\"cluster-directory\"].path)'"
           "-e" "numberFile='$(inputs[\"number-file\"].path)'"
           "-e" "numberOfImages='$(inputs[\"images-number\"])'"
           "-e" "outputDirectory='$(inputs[\"output-directory\"])'"
           "-e" "stdpExecutable='$(inputs[\"stdp-executable\"].path)'"
           gnuplot-script
           #:outputs
           (image-directory #:type Directory
                            #:binding '((glob . "$(inputs[\"output-directory\"])")))))

(workflow ((stdp-executable #:type File)
           (response-test #:type File)
           (lateral-weight #:type File)
           (text-image-directory #:type Directory)

           (excitatory-neuron-number #:type int #:default 100)
           (inhibitory-neuron-number #:type int #:default 20)
           (test-stimulation-number #:type int #:default 1000)
           (correlation-threshold-neuron #:type float #:default 0.9)
           (minimum-cluster-size-neuron #:type int #:default 10)
           (correlation-threshold-stimulation #:type float #:default 0.9)
           (minimum-cluster-size-stimulation #:type int #:default 10)
           (images-number #:type int #:default 109999)

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

           (output-cluster-images-directory #:type string #:default "clusterImages"))
          (pipe
           (tee
            (lateral-weight-cut-out-excitatory
             #:stdp-executable stdp-executable
             #:lateral-weight lateral-weight
             #:excitatory-neuron-number excitatory-neuron-number
             #:inhibitory-neuron-number inhibitory-neuron-number)
            (pipe (response-cut-out-excitatory
                   #:stdp-executable stdp-executable
                   #:response response-test
                   #:excitatory-neuron-number excitatory-neuron-number
                   #:inhibitory-neuron-number inhibitory-neuron-number
                   #:stimulation-number test-stimulation-number)
                  (response-clustering
                   #:stdp-executable stdp-executable
                   #:response response-excitatory
                   #:neuron-number excitatory-neuron-number
                   #:stimulation-number test-stimulation-number)))
           (tee
            (pipe
             (sort-lateral-weight
              #:stdp-executable stdp-executable
              #:weight-excitatory weight-excitatory
              #:sort-index-neuron-row sort-index-neuron
              #:sort-index-neuron-colomn sort-index-neuron
              #:output-name output-weight-sorted-txt)
             (plot-matrix (plot-weight)
              #:gnuplot-script matrix-plot-script
              #:matrix weight-sorted
              #:output-name output-weight-sorted-svg
              #:title title-weight-sorted))
            (pipe
             (response-correlation-matrix
              #:stdp-executable stdp-executable
              #:response response-sorted
              #:neuron-number excitatory-neuron-number
              #:stimulation-number test-stimulation-number)
             (tee
              (plot-correlation-neuron
               #:gnuplot-script correlation-plot-script
               #:correlation-matrix correlation-matrix-neuron
               #:output-name output-correlation-matrix-neuron
               #:title title-correlation-matrix-neuron)
              (plot-correlation-stimulation
               #:gnuplot-script correlation-plot-script-without-pixels
               #:correlation-matrix correlation-matrix-stimulation
               #:output-name output-correlation-matrix-stimulation
               #:title title-correlation-matrix-stimulation)
              (pipe
               (cluster-map (cluster-map-neuron)
                            #:stdp-executable stdp-executable
                            #:correlation-matrix correlation-matrix-neuron
                            #:input-size excitatory-neuron-number
                            #:correlation-threshold correlation-threshold-neuron
                            #:minimum-cluster-size minimum-cluster-size-neuron
                            #:output-name output-cluster-map-neuron)
               (divide-line (divide-line-neuron)
                            #:stdp-executable stdp-executable
                            #:cluster-map cluster-map
                            #:output-directory output-directory-cluster-map-neuron
                            #:output-number-file output-number-cluster-map-neuron)
               (rename #:divided-directory-neuron divided-directory
                       #:cluster-number-neuron output-number-file))
              (pipe
               (cluster-map (cluster-map-stimulation)
                            #:stdp-executable stdp-executable
                            #:correlation-matrix correlation-matrix-stimulation
                            #:input-size test-stimulation-number
                            #:correlation-threshold correlation-threshold-stimulation
                            #:minimum-cluster-size minimum-cluster-size-stimulation
                            #:output-name output-cluster-map-stimulation)
               (divide-line (divide-line-stimulation)
                            #:stdp-executable stdp-executable
                            #:cluster-map cluster-map
                            #:output-directory output-directory-cluster-map-stimulation
                            #:output-number-file output-number-cluster-map-stimulation)
               (rename #:divided-directory-stimulation divided-directory
                       #:cluster-number-file-stimulation number-file)
               (each-cluster-images
                #:gnuplot-script each-cluster-images-script
                #:stdp-executable stdp-executable
                #:text-image-directory text-image-directory
                #:cluster-directory divided-directory-stimulation
                #:number-file cluster-number-file-stimulation
                #:images-number images-number
                #:output-directory output-cluster-images-directory)
               ))))))