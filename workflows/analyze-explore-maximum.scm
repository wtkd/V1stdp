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
           (table-file #:type File)
           (output-file #:type string)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/line-chart.gnuplot")))
           #:run
           "gnuplot"
           "-e" "inputFile='$(inputs[\"table-file\"].path)'"
           "-e" "outputFile='$(inputs[\"output-file\"])'"
           "-e" "title='$(inputs[\"title\"])'"
           gnuplot-script
           #:outputs
           (output-plot
            #:type File
            #:binding ((glob . "$(inputs[\"output-file\"])")))))

(define generate-line-point-evaluation-plot
  (command #:inputs
           (title #:type string)
           (table-file #:type File)
           (output-file #:type string)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/line-chart-evaluation.gnuplot")))
           #:run
           "gnuplot"
           "-e" "inputFile='$(inputs[\"table-file\"].path)'"
           "-e" "outputFile='$(inputs[\"output-file\"])'"
           "-e" "title='$(inputs[\"title\"])'"
           gnuplot-script
           #:outputs
           (output-plot
            #:type File
            #:binding ((glob . "$(inputs[\"output-file\"])")))))

(define plot-image
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
                        #:binding ((glob . "$(inputs[\"output-name\"])")))))

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

(define convert-svg-to-png
  (command #:inputs
           (input-svg #:type File)
           (output-png-name #:type string)
           #:run
           "rsvg-convert" input-svg "-o" output-png-name "-b" "white"
           #:outputs
           (output-png #:type File
                       #:binding ((glob . "$(inputs[\"output-png-name\"])")))))

(workflow ((stdp-executable #:type File)
           (input-directory #:type Directory)
           (evaluations-file #:type File)
           (correlation-file #:type File)
           (sparseness-file #:type File)
           (smoothness-file #:type File)
           (response-file #:type File)
           (active-activity-file #:type File)
           (inactive-activity-file #:type File)
           (text-image #:type File)
           (template-response #:type File)
           (total-iteration-number #:type int)
           (neuron-number #:type int)
           (delta #:type float)
           (sort-index-neuron #:type File)
           (interval #:type int #:default 10))
          (pipe
           (tee
            (generate-explored-images
             #:input-directory input-directory
             #:total-iteration-number total-iteration-number
             #:delta delta
             #:output-file "all-explored.svg"
             #:interval interval)
            (pipe
             (generate-line-point-evaluation-plot
              #:table-file evaluations-file
              #:title "Evaluation of each iteration"
              #:output-file "evaluations.svg")
             (rename #:output-evaluations-plot output-plot))
            (pipe
             (generate-line-point-plot (generate-correlation-plot)
              #:table-file correlation-file
              #:title "Correlation of each iteration"
              #:output-file "correlation.svg")
             (rename #:output-correlation-plot output-plot))
            (pipe
             (generate-line-point-plot (generate-sparseness-plot)
              #:table-file sparseness-file
              #:title "Sparseness of each iteration"
              #:output-file "sparseness.svg")
             (rename #:output-sparseness-plot output-plot))
            (pipe
             (generate-line-point-plot (generate-smoothness-plot)
              #:table-file smoothness-file
              #:title "Smoothness of each iteration"
              #:output-file "smoothness.svg")
             (rename #:output-smoothness-plot output-plot))
            (pipe
             (generate-line-point-plot (generate-active-activity-plot)
              #:table-file active-activity-file
              #:title "Should-be-active neuron activity of each iteration"
              #:output-file "active-activity.svg")
             (rename #:output-active-activity-plot output-plot))
            (pipe
             (generate-line-point-plot (generate-inactive-activity-plot)
              #:table-file inactive-activity-file
              #:title "Should-be-inactive neuron activity of each iteration"
              #:output-file "inactive-activity.svg")
             (rename #:output-inactive-activity-plot output-plot))
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
             (rename #:responses-plot matrix-plot))
            (pipe
             (plot-image
              #:matrix text-image
              #:title "Result image of exploring"
              #:output-name "result-image.svg")
             (rename #:result-image-plot matrix-plot)))
           (tee
            (identity)
            (pipe
             (convert-svg-to-png (convert-svg-to-png-response)
              #:input-svg responses-plot
              #:output-png-name "responses.png")
             (rename #:output-responses-plot-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-evaluation)
              #:input-svg output-evaluations-plot
              #:output-png-name "evaluations.png")
             (rename #:output-evaluations-plot-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-correlation)
              #:input-svg output-correlation-plot
              #:output-png-name "correlation.png")
             (rename #:output-correlation-plot-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-sparseness)
              #:input-svg output-sparseness-plot
              #:output-png-name "sparseness.png")
             (rename #:output-sparseness-plot-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-smoothness)
              #:input-svg output-smoothness-plot
              #:output-png-name "smoothness.png")
             (rename #:output-smoothness-plot-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-active-activity)
              #:input-svg output-active-activity-plot
              #:output-png-name "active-activity.png")
             (rename #:output-active-activity-plot-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-inactive-activity)
              #:input-svg output-inactive-activity-plot
              #:output-png-name "inactive-activity.png")
             (rename #:output-inactive-activity-plot-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-explored)
              #:input-svg output-explored
              #:output-png-name "all-explored.png")
             (rename #:output-explored-png output-png))
            (pipe
             (convert-svg-to-png (convert-svg-to-png-result-image)
              #:input-svg result-image-plot
              #:output-png-name "result-image.png")
             (rename #:output-result-image-plot-png output-png)))))
