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

           ;; For export-images
           (transformed-image-data #:type File
                                   #:default '((class . "File")
                                               (location . "../patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat")))
           (total-image-number #:type int #:default 109999)
           (edge-length #:type int #:default 17))
          (pipe (tee
                 (learn #:stdp-executable stdp-executable
                        #:transformed-image-data transformed-image-data
                        #:step step-learn
                        #:seed seed
                        #:step-test step-test)
                 (export-images
                  #:stdp-executable stdp-executable
                  #:transformed-image-data transformed-image-data
                  #:total-image-number total-image-number
                  #:edge-length edge-length))
                (tee
                 (rename #:lateral-weight lateral-weight-txt
                         #:feedforward-weight feedforward-weight-txt)
                 (test #:stdp-executable stdp-executable
                       #:transformed-image-data transformed-image-data
                       #:step step-test
                       #:seed seed
                       #:lateral-weight-dat lateral-weight-dat
                       #:feedforward-weight-dat feedforward-weight-dat))
                (analyze
                 #:stdp-executable stdp-executable
                 #:response-test resps-test
                 #:lateral-weight lateral-weight
                 #:feedforward-weight feedforward-weight
                 #:text-image-directory text-images-directory
                 #:excitatory-neuron-number excitatory-neuron-number
                 #:inhibitory-neuron-number inhibitory-neuron-number
                 #:test-stimulation-number step-test
                 #:correlation-threshold-neuron correlation-threshold-neuron
                 #:minimum-cluster-size-neuron minimum-cluster-size-neuron
                 #:correlation-threshold-stimulation correlation-threshold-stimulation
                 #:minimum-cluster-size-stimulation minimum-cluster-size-stimulation
                 #:total-image-number total-image-number)))
