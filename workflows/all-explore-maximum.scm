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
           (delta #:type float))
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
            #:delta delta)
           (tee
            (rename #:ouptut-text-image ouptut-text-image
                    #:ouptut-text-image-log ouptut-text-image-log)
            (analyze-explore-maximum
             #:input-directory ouptut-text-image-log
             #:total-iteration-number total-iteration-number
             #:delta delta))))
