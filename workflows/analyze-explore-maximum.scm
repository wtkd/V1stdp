(define generate-svg-image
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

(workflow ((input-directory #:type Directory)
           (total-iteration-number #:type int)
           (delta #:type float)
           (interval #:type int #:default 10))
          (generate-svg-image
           #:input-directory input-directory
           #:total-iteration-number total-iteration-number
           #:delta delta
           #:output-file "all-explored.svg"
           #:interval interval))
