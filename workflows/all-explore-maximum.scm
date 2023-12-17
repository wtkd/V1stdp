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
           (evaluation-function-parameter-sparseness-width #:type float)
           (evaluation-function-parameter-smoothness-intensity #:type float)
           (evaluation-function-parameter-standard-derivation-intensity #:type float)
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
            #:evaluation-function-parameter-sparseness-width evaluation-function-parameter-sparseness-width
            #:evaluation-function-parameter-smoothness-intensity evaluation-function-parameter-smoothness-intensity
            #:evaluation-function-parameter-standard-derivation-intensity evaluation-function-parameter-standard-derivation-intensity)
           (tee
            (identity)
            (analyze-explore-maximum
             #:stdp-executable stdp-executable
             #:input-directory output-text-image-log
             #:total-iteration-number total-iteration-number
             #:delta delta
             #:evaluations-file output-evaluations-file
             #:correlation-file output-correlation-file
             #:sparseness-file output-sparseness-file
             #:smoothness-file output-smoothness-file
             #:standard-derivation-file output-standard-derivation-file
             #:active-activity-file output-active-activity-file
             #:inactive-activity-file output-inactive-activity-file
             #:response-file output-response-file
             #:text-image output-text-image
             #:sort-index-neuron sort-index-neuron
             #:template-response template-response
             #:neuron-number neuron-number))))
