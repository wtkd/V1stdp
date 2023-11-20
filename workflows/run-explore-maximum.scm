(define run-explore-maximum
  (command #:inputs
           (stdp-executable #:type File)
           (transformed-image-data #:type File)
           (neuron-number #:type int)
           (initial-input-number #:type int)
           (total-iteration-number #:type int)
           (lateral-weight #:type File)
           (feedforward-weight #:type File)
           (delays #:type File)
           (output-text-image-name #:type string)
           (output-log-directory-name #:type string)
           (template-response #:type File)
           (delta #:type float)
           #:run
           stdp-executable "tool" "analyze" "explore-maximum"
           "--neuron-number" neuron-number
           "--input-file" transformed-image-data
           "--initial-input-number" initial-input-number
           "--iteration-number" total-iteration-number
           "--save-directory" "tmp"
           "--output-file" output-text-image-name
           "--lateral-weight" lateral-weight
           "--feedforward-weight" feedforward-weight
           "--template-response" template-response
           "--delays-file" delays
           "--delta" delta
           "--save-log-directory" output-log-directory-name
           "--save-log-interval" "1"
           "--save-evaluation-file" "evaluations.txt"
           "--save-evaluation-pixel-file" "evaluations-pixel.txt"
           #:outputs
           (output-text-image
            #:type File
            #:binding ((glob . "$(inputs[\"output-text-image-name\"])")))
           (ouptut-text-image-log
            #:type Directory
            #:binding ((glob . "$(inputs[\"output-log-directory-name\"])")))
           (output-evaluation-file
            #:type File
            #:binding ((glob . "evaluations.txt")))
           (output-evaluation-pixel-file
            #:type File
            #:binding ((glob . "evaluations-pixel.txt")))))

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
          (run-explore-maximum
           #:stdp-executable stdp-executable
           #:transformed-image-data transformed-image-data
           #:neuron-number neuron-number
           #:initial-input-number initial-input-number
           #:total-iteration-number total-iteration-number
           #:lateral-weight lateral-weight
           #:feedforward-weight feedforward-weight
           #:delays delays
           #:output-text-image-name "explored-image.txt"
           #:output-log-directory-name "explored-log"
           #:template-response template-response
           #:delta delta))
