(define generate-explored-images
  (command #:inputs
           (input-directory #:type Directory)
           (total-iteration-number #:type int)
           (output-file #:type string)
           (delta #:type float)
           (interval #:type int)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/showExploredImages.gnuplot")))
           #:run
           "gnuplot"
           "-e" "inputDirectory='$(inputs[\"input-directory\"].path)'"
           "-e" "outputFile='$(inputs[\"output-file\"])'"
           "-e" "totalIterationNumber=$(inputs[\"total-iteration-number\"])"
           "-e" "title='Input of each iterations on exploring (delta=$(inputs.delta), n=$(inputs[\"total-iteration-number\"]))'"
           "-e" "interval=$(inputs[\"interval\"])"
           gnuplot-script
           #:outputs
           (output-explored
            #:type File
            #:binding ((glob . "$(inputs[\"output-file\"])")))))

(define generate-line-point-plot
  (command #:inputs
           (title #:type string)
           (evaluations-file #:type File)
           (output-file #:type string)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/line-chart.gnuplot")))
           #:run
           "gnuplot"
           "-e" "inputFile='$(inputs[\"evaluations-file\"].path)'"
           "-e" "outputFile='$(inputs[\"output-file\"])'"
           "-e" "title='$(inputs[\"title\"])'"
           gnuplot-script
           #:outputs
           (output-evaluation-plot
            #:type File
            #:binding ((glob . "$(inputs[\"output-file\"])")))))

(define sort-response-neuron
  (command #:inputs
           (stdp-executable #:type File)
           (response #:type File)
           (sort-index-neuron #:type File)
           (output-name #:type string)
           #:run
           stdp-executable "tool" "analyze" "apply-permutation"
           response output-name
           "--row" sort-index-neuron
           #:outputs
           (response-sorted-neuron #:type File
                                   #:binding ((glob . "$(inputs[\"output-name\"])")))))

(define transpose-response
  (command #:inputs
           (stdp-executable #:type File)
           (input #:type File)
           (output-name #:type string)
           (colomn #:type int)
           #:run
           stdp-executable "tool" "analyze" "transpose"
           input output-name
           "--colomn" colomn
           #:outputs
           (output #:type File
                   #:binding ((glob . "$(inputs[\"output-name\"])")))))

(define plot-matrix-with-template
  (command #:inputs
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/matrix-with-template.gnuplot")))
           (matrix #:type File)
           (template #:type File)
           (output-name #:type string)
           (title #:type string)
           #:run
           "gnuplot"
           "-e" "inputFile='$(inputs.matrix.path)'"
           "-e" "outputFile='$(inputs[\"output-name\"])'"
           "-e" "title='$(inputs.title)'"
           "-e" "template='$(inputs.template.path)'"
           gnuplot-script
           #:outputs
           (matrix-plot #:type File
                        #:binding ((glob . "$(inputs[\"output-name\"])")))))

(workflow ((stdp-executable #:type File)
           (input-directory #:type Directory)
           (evaluations-file #:type File)
           (response-file #:type File)
           (template-response #:type File)
           (total-iteration-number #:type int)
           (neuron-number #:type int)
           (delta #:type float)
           (sort-index-neuron #:type File)
           (interval #:type int #:default 10))
          (tee
           (generate-explored-images
            #:input-directory input-directory
            #:total-iteration-number total-iteration-number
            #:delta delta
            #:output-file "all-explored.svg"
            #:interval interval)
           (generate-line-point-plot
            #:evaluations-file evaluations-file
            #:title "Evaluation of each iteration"
            #:output-file "evaluations.svg")
           ;; (pipe
           ;;  (sort-response-neuron (sort-template-responses)
           ;;                        #:stdp-executable stdp-executable
           ;;                        #:response template-response
           ;;                        #:sort-index-neuron sort-index-neuron
           ;;                        #:output-name "template-response-sorted-neuron.txt")
           ;;  (plot-matrix (plot-template-response)
           ;;               #:matrix response-sorted-neuron
           ;;               #:title "Template response"
           ;;               #:output-name "template-reseponse.svg")
           ;;  (rename #:template-response-plot matrix-plot))
           (pipe
            (tee
             (pipe
              (transpose-response
               #:stdp-executable stdp-executable
               #:input response-file
               #:output-name "response-sorted-transposed.txt"
               #:colomn neuron-number)
              (sort-response-neuron (sort-responses)
                                    #:stdp-executable stdp-executable
                                    #:response output
                                    #:sort-index-neuron sort-index-neuron
                                    #:output-name "response-sorted-neuron.txt"))
             (pipe
              (sort-response-neuron (sort-template-responses)
                                    #:stdp-executable stdp-executable
                                    #:response template-response
                                    #:sort-index-neuron sort-index-neuron
                                    #:output-name "template-response-sorted-neuron.txt")
              (rename #:template-response-sorted response-sorted-neuron)))
            (plot-matrix-with-template
             #:matrix response-sorted-neuron
             #:template template-response-sorted
             #:title "Reponse of each iteration"
             #:output-name "responses.svg")
            (rename #:responses-plot matrix-plot))))
