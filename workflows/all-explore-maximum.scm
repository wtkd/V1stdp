(define analyze-explore-maximum
  (cwl-workflow "analyze-explore-maximum.cwl"))

(define run-explore-maximum
  (cwl-workflow "run-explore-maximum.cwl"))

(workflow ((stdp-executable #:type File)
           (transformed-image-data #:type File)
           (neuron-number #:type int #:default 100)
           (initial-input-number #:type int)
           (total-iteration-number #:type int)
           (lateral-weight #:type File)
           (feedforward-weight #:type File)
           (delays #:type File)
           (template-response #:type File)
           (delta #:type float)
           (evaluation-function-parameter-a #:type float)
           (evaluation-function-parameter-b #:type float)
           (evaluation-function-parameter-sparseness-intensity #:type float)
           (evaluation-function-parameter-sparseness-range #:type float)
           (sort-index-neuron #:type File))
          (pipe
           (run-explore-maximum
            #:stdp-executable stdp-executable
            #:transformed-image-data transformed-image-data
            #:neuron-number neuron-number
            #:initial-input-number initial-input-number
            #:total-iteration-number total-iteration-number
            #:lateral-weight lateral-weight
            #:feedforward-weight feedforward-weight
            #:delays delays
            #:template-response template-response
            #:delta delta
            #:evaluation-function-parameter-a evaluation-function-parameter-a
            #:evaluation-function-parameter-b evaluation-function-parameter-b
            #:evaluation-function-parameter-sparseness-intensity evaluation-function-parameter-sparseness-intensity
            #:evaluation-function-parameter-sparseness-range evaluation-function-parameter-sparseness-range)
           (tee
            (rename #:output-text-image output-text-image
                    #:output-text-image-log output-text-image-log
                    #:output-evaluation-file output-evaluation-file
                    #:output-evaluation-pixel-file output-evaluation-pixel-file
                    #:output-response-file output-response-file)
            (analyze-explore-maximum
             #:stdp-executable stdp-executable
             #:input-directory output-text-image-log
             #:evaluations-file output-evaluation-file
             #:total-iteration-number total-iteration-number
             #:delta delta
             #:response-file output-response-file
             #:sort-index-neuron sort-index-neuron
             #:template-response template-response
             #:neuron-number neuron-number))))
