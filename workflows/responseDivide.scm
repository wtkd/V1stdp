(define transpose-response
  (command #:inputs
           (stdp-executable #:type File)
           (input #:type File)
           (output-name #:type string)
           (total-neuron-number #:type int)
           (total-stimulation-number #:type int)
           #:run
           stdp-executable "tool" "analyze" "transpose"
           input output-name
           "--colomn" total-stimulation-number
           "--row" total-neuron-number
           #:outputs
           (output #:type File
                   #:binding ((glob . "$(inputs[\"output-name\"])")))))

(define divide-line
  (command #:inputs
           (stdp-executable #:type File)
           (response-transposed #:type File)
           (output-directory #:type string)
           #:run
           stdp-executable "tool" "analyze" "divide-line"
           "--index-begin" "1"
           response-transposed output-directory
           #:outputs
           (divided-directory #:type Directory
                              #:binding ((glob . "$(inputs[\"output-directory\"])")))))

(workflow ((stdp-executable #:type File)
           (response-matrix #:type File)
           (total-neuron-number #:type int)
           (total-stimulation-number #:type int))
          (pipe
           (transpose-response
            #:stdp-executable stdp-executable
            #:input response-matrix
            #:output-name "response-transposed.txt"
            #:total-neuron-number total-neuron-number
            #:total-stimulation-number total-stimulation-number)
           (divide-line
            #:stdp-executable stdp-executable
            #:response-transposed output
            #:output-directory "eachResponse")))
