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

(define plot-correlation
  (command #:inputs
           (gnuplot-script
            #:type File
            #:default '((class . "File")
                        (location . "../script/correlation.gnuplot")))
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
           (correlation-plot #:type File
                             #:binding '((glob . "$(inputs[\"output-name\"])")))))

(define plot-correlation-without-pixels
  (command #:inputs
           (gnuplot-script
            #:type File
            #:default '((class . "File")
                        (location . "../script/correlation-without-pixels.gnuplot")))
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
           (correlation-plot #:type File
                             #:binding '((glob . "$(inputs[\"output-name\"])")))))

(define plot-matrix
  (command #:inputs
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/matrix.gnuplot")))
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

(define export-feedforward-weight
  (command #:inputs
           (stdp-executable #:type File)
           (feedforward-weight #:type File)
           (on-directory-name #:type string)
           (off-directory-name #:type string)
           (diff-directory-name #:type string)
           (excitatory-neuron-number #:type int)
           (inhibitory-neuron-number #:type int)
           (edge-length #:type int)
           #:run
           stdp-executable
           "tool" "analyze" "weight" "feedforward" "export"
           feedforward-weight
           "--on-center-directory" on-directory-name
           "--off-center-directory" off-directory-name
           "--diff-directory" diff-directory-name
           "--edge-length" edge-length
           "--excitatory-neuron-number" excitatory-neuron-number
           "--inhibitory-neuron-number" inhibitory-neuron-number
           "--zero-padding" "0"
           #:outputs
           (on-feedforward-weight #:type Directory
                                  #:binding '((glob . "$(inputs[\"on-directory-name\"])")))
           (off-feedforward-weight #:type Directory
                                   #:binding '((glob . "$(inputs[\"off-directory-name\"])")))
           (diff-feedforward-weight #:type Directory
                                    #:binding '((glob . "$(inputs[\"diff-directory-name\"])")))))

(define plot-each-feedforward-weight
  (command #:inputs
           (stdp-executable #:type File)
           (neuron-number #:type int)
           (on-feedforward-text #:type Directory)
           (off-feedforward-text #:type Directory)
           (diff-feedforward-text #:type Directory)
           (output-directory #:type string)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/each-feedforward-weight.gnuplot")))
           #:run
           "gnuplot"
           "-e" "n=$(inputs[\"neuron-number\"])"
           "-e" "onInputDirectory='$(inputs[\"on-feedforward-text\"].path)'"
           "-e" "offInputDirectory='$(inputs[\"off-feedforward-text\"].path)'"
           "-e" "diffInputDirectory='$(inputs[\"diff-feedforward-text\"].path)'"
           "-e" "outputDirectory='$(inputs[\"output-directory\"])'"
           "-e" "stdpExecutable='$(inputs[\"stdp-executable\"].path)'"
           gnuplot-script
           #:outputs
           (feedforward-weight-plots-directory
            #:type Directory
            #:binding '((glob . "$(inputs[\"output-directory\"])")))))

(define plot-all-feedforward-weight
  (command #:inputs
           (stdp-executable #:type File)
           (neuron-number #:type int)
           (diff-feedforward-text #:type Directory)
           (output-file #:type string)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/all-feedforward-weight.gnuplot")))
           #:run
           "gnuplot"
           "-e" "n=$(inputs[\"neuron-number\"])"
           "-e" "diffInputDirectory='$(inputs[\"diff-feedforward-text\"].path)'"
           "-e" "outputFile='$(inputs[\"output-file\"])'"
           gnuplot-script
           #:outputs
           (feedforward-weight-plot #:type File
                                    #:binding '((glob . "$(inputs[\"output-file\"])")))))

(define plot-each-cluster-images
  (command #:inputs
           (stdp-executable #:type File)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/each-cluster-images.gnuplot")))
           (text-image-directory #:type Directory)
           (cluster-directory #:type Directory)
           (number-file #:type File)
           (image-range #:type int)
           (total-image-number #:type int)
           (output-directory #:type string)
           #:run
           "gnuplot"
           "-e" "textImageDirectory='$(inputs[\"text-image-directory\"].path)'"
           "-e" "clusterDirectory='$(inputs[\"cluster-directory\"].path)'"
           "-e" "numberFile='$(inputs[\"number-file\"].path)'"
           "-e" "totalImageNumber='$(inputs[\"total-image-number\"])'"
           "-e" "imageRange='$(inputs[\"image-range\"])'"
           "-e" "outputDirectory='$(inputs[\"output-directory\"])'"
           "-e" "stdpExecutable='$(inputs[\"stdp-executable\"].path)'"
           gnuplot-script
           #:outputs
           (image-directory #:type Directory
                            #:binding '((glob . "$(inputs[\"output-directory\"])")))))

(define convert-svg-to-png
  (command #:inputs
           (input-svg #:type File)
           (output-png-name #:type string)
           #:run
           "rsvg-convert" input-svg "-o" output-png-name "-b" "white"
           #:outputs
           (output-png #:type File
                       #:binding '((glob . "$(inputs[\"output-png-name\"])")))))

(define convert-svg-to-png-directory
  (command #:inputs
           (stdp-executable #:type File)
           (input-directory #:type Directory)
           (output-directory-name #:type string)
           #:run
           stdp-executable "tool" "runner" "rsvg-convert"
           input-directory output-directory-name
           #:outputs
           (output-directory #:type Directory
                             #:binding '((glob . "$(inputs[\"output-directory-name\"])")))))

(workflow ((stdp-executable #:type File)
           (response-test #:type File)
           (lateral-weight #:type File)
           (feedforward-weight #:type File)
           (text-image-directory #:type Directory)

           (excitatory-neuron-number #:type int #:default 100)
           (inhibitory-neuron-number #:type int #:default 20)
           (test-stimulation-number #:type int #:default 1000)
           (image-range #:type int #:default 1000)
           (correlation-threshold-neuron #:type float #:default 0.9)
           (minimum-cluster-size-neuron #:type int #:default 10)
           (correlation-threshold-stimulation #:type float #:default 0.9)
           (minimum-cluster-size-stimulation #:type int #:default 10)
           (total-image-number #:type int #:default 109999)
           (edge-length #:type int #:default 17)

           (output-on-feedforward-weight #:type string #:default "feedforward-weight-on")
           (output-off-feedforward-weight #:type string #:default "feedforward-weight-off")
           (output-diff-feedforward-weight #:type string #:default "feedforward-weight-diff")

           (output-feedforward-weight-images #:type string #:default "feedforward-weight-images")
           (output-feedforward-weight-image-svg #:type string #:default "feedforward-weight-image.svg")
           (output-feedforward-weight-image-png #:type string #:default "feedforward-weight-image.png")

           (output-response-svg #:type string #:default "response-sorted.svg")
           (title-response-svg #:type string #:default "Response of excitatory neurons on each stimulation")

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
           (output-number-cluster-map-stimulation #:type string #:default "cluster-map-stimulation-number.txt")

           (output-cluster-images-directory #:type string #:default "clusterImages")
           (output-cluster-images-directory-png #:type string #:default "clusterImagesPng"))
          (tee
           (pipe
            (export-feedforward-weight
             #:stdp-executable stdp-executable
             #:feedforward-weight feedforward-weight
             #:on-directory-name output-on-feedforward-weight
             #:off-directory-name output-off-feedforward-weight
             #:diff-directory-name output-diff-feedforward-weight
             #:excitatory-neuron-number excitatory-neuron-number
             #:inhibitory-neuron-number inhibitory-neuron-number
             #:edge-length edge-length)
            (tee
             (rename
              #:on-feedforward-weight on-feedforward-weight
              #:off-feedforward-weight off-feedforward-weight
              #:diff-feedforward-weight diff-feedforward-weight)
             (plot-each-feedforward-weight
              #:stdp-executable stdp-executable
              #:neuron-number excitatory-neuron-number
              #:on-feedforward-text on-feedforward-weight
              #:off-feedforward-text off-feedforward-weight
              #:diff-feedforward-text diff-feedforward-weight
              #:output-directory output-feedforward-weight-images)
             (pipe
              (plot-all-feedforward-weight
               #:stdp-executable stdp-executable
               #:neuron-number excitatory-neuron-number
               #:diff-feedforward-text diff-feedforward-weight
               #:output-file output-feedforward-weight-image-svg)
              (tee
               (rename #:feedforward-weight-plot-svg feedforward-weight-plot)
               (pipe
                (convert-svg-to-png
                 #:input-svg feedforward-weight-plot
                 #:output-png-name output-feedforward-weight-image-png)
                (rename #:feedforward-weight-plot-png output-png))))))
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
              (plot-matrix (plot-response)
                           #:matrix response-sorted
                           #:output-name output-response-svg
                           #:title title-response-svg)
              (rename #:plot-response-matrix matrix-plot))
             (pipe
              (sort-lateral-weight
               #:stdp-executable stdp-executable
               #:weight-excitatory weight-excitatory
               #:sort-index-neuron-row sort-index-neuron
               #:sort-index-neuron-colomn sort-index-neuron
               #:output-name output-weight-sorted-txt)
              (plot-matrix (plot-weight)
                           #:matrix weight-sorted
                           #:output-name output-weight-sorted-svg
                           #:title title-weight-sorted)
              (rename #:plot-weight-matrix matrix-plot))
             (pipe
              (response-correlation-matrix
               #:stdp-executable stdp-executable
               #:response response-sorted
               #:neuron-number excitatory-neuron-number
               #:stimulation-number test-stimulation-number)
              (tee
               (pipe
                (plot-correlation
                 #:correlation-matrix correlation-matrix-neuron
                 #:output-name output-correlation-matrix-neuron
                 #:title title-correlation-matrix-neuron)
                (rename #:correlation-plot-neuron correlation-plot))
               (pipe
                (plot-correlation-without-pixels
                 #:correlation-matrix correlation-matrix-stimulation
                 #:output-name output-correlation-matrix-stimulation
                 #:title title-correlation-matrix-stimulation)
                (rename #:correlation-plot-stimulation correlation-plot))
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
                        #:cluster-number-neuron number-file))
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
                (plot-each-cluster-images
                 #:stdp-executable stdp-executable
                 #:text-image-directory text-image-directory
                 #:cluster-directory divided-directory-stimulation
                 #:number-file cluster-number-file-stimulation
                 #:total-image-number total-image-number
                 #:image-range image-range
                 #:output-directory output-cluster-images-directory)
                (tee
                 (rename #:each-cluster-images image-directory)
                 (convert-svg-to-png-directory
                  #:stdp-executable stdp-executable
                  #:input-directory image-directory
                  #:output-directory-name output-cluster-images-directory-png)))))))))
