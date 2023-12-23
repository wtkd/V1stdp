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
           (evaluation-function-parameter-a #:type float)
           (evaluation-function-parameter-b #:type float)
           (evaluation-function-parameter-sparseness-intensity #:type float)
           (evaluation-function-parameter-sparseness-width #:type float)
           (evaluation-function-parameter-smoothness-intensity #:type float)
           (evaluation-function-parameter-standard-derivation-intensity #:type float)
           #:run
           stdp-executable "tool" "analyze" "explore-maximum"
           "--neuron-number" neuron-number
           "--input-file" transformed-image-data
           "--initial-input-number" initial-input-number
           "--iteration-number" total-iteration-number
           "--output-file" output-text-image-name
           "--lateral-weight" lateral-weight
           "--feedforward-weight" feedforward-weight
           "--template-response" template-response
           "--delays-file" delays
           "--gradient-descent"
           "--delta" delta
           "--save-log-directory" output-log-directory-name
           "--save-log-interval" "1"
           "--save-evaluation-file" "evaluations.txt"
           "--save-evaluation-pixel-file" "evaluations-pixel.txt"
           "--save-correlation-file" "correlation.txt"
           "--save-sparseness-file" "sparseness.txt"
           "--save-smoothness-file" "smoothness.txt"
           "--save-standard-derivation-file" "standard-derivation.txt"
           "--save-response-file" "responses.txt"
           "--save-active-neuron-activity-file" "activity-active.txt"
           "--save-inactive-neuron-activity-file" "activity-inactive.txt"
           "--evaluation-function-parameter-a" evaluation-function-parameter-a
           "--evaluation-function-parameter-b" evaluation-function-parameter-b
           "--evaluation-function-parameter-sparseness-intensity" evaluation-function-parameter-sparseness-intensity
           "--evaluation-function-parameter-sparseness-width" evaluation-function-parameter-sparseness-width
           "--evaluation-function-parameter-smoothness-intensity" evaluation-function-parameter-smoothness-intensity
           "--evaluation-function-parameter-standard-derivation-intensity" evaluation-function-parameter-standard-derivation-intensity
           #:outputs
           (output-text-image
            #:type File
            #:binding ((glob . "$(inputs[\"output-text-image-name\"])")))
           (output-text-image-log
            #:type Directory
            #:binding ((glob . "$(inputs[\"output-log-directory-name\"])")))
           (output-evaluations-file
            #:type File
            #:binding ((glob . "evaluations.txt")))
           (output-evaluation-pixel-file
            #:type File
            #:binding ((glob . "evaluations-pixel.txt")))
           (output-correlation-file
            #:type File
            #:binding ((glob . "correlation.txt")))
           (output-sparseness-file
            #:type File
            #:binding ((glob . "sparseness.txt")))
           (output-smoothness-file
            #:type File
            #:binding ((glob . "smoothness.txt")))
           (output-standard-derivation-file
            #:type File
            #:binding ((glob . "standard-derivation.txt")))
           (output-response-file
            #:type File
            #:binding ((glob . "responses.txt")))
           (output-active-activity-file
            #:type File
            #:binding ((glob . "activity-active.txt")))
           (output-inactive-activity-file
            #:type File
            #:binding ((glob . "activity-inactive.txt")))))

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
           (evaluation-function-parameter-standard-derivation-intensity #:type float))
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
           #:delta delta
           #:evaluation-function-parameter-a evaluation-function-parameter-a
           #:evaluation-function-parameter-b evaluation-function-parameter-b
           #:evaluation-function-parameter-sparseness-intensity evaluation-function-parameter-sparseness-intensity
           #:evaluation-function-parameter-sparseness-width evaluation-function-parameter-sparseness-width
           #:evaluation-function-parameter-smoothness-intensity evaluation-function-parameter-smoothness-intensity
           #:evaluation-function-parameter-standard-derivation-intensity evaluation-function-parameter-standard-derivation-intensity))
