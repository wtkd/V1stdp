(define learn
  (command #:inputs
           (stdp-executable #:type File)
           (transformed-image-data #:type File)
           (step #:type int)
           (seed #:type int)
           (step-test #:type int #:default 1000)
           #:run
           stdp-executable "learn"
           "--save-directory" "."
           "--input-file" transformed-image-data
           "--step" step
           "--seed" seed
           ;; To exclude input images for test
           "--image-range" "-$(inputs[\"step-test\"])"
           #:outputs
           (log #:type stdout)
           (lateral-weight-txt
            #:type File
            #:binding ((glob . "w.txt")))
           (lateral-weight-dat
            #:type File
            #:binding ((glob . "w.dat")))
           (feedforward-weight-txt
            #:type File
            #:binding ((glob . "wff.txt")))
           (feedforward-weight-dat
            #:type File
            #:binding ((glob . "wff.dat")))))

(workflow ((stdp-executable #:type File
                            #:default '((class . "File")
                                        (location . "../build/src/main/stdp")))
           (transformed-image-data #:type File
                                   #:default '((class . "File")
                                               (location . "../patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat")))
           (step #:type int #:default 1000000)
           (seed #:type int #:default 0)
           (step-test #:type int #:default 1000))
          (learn #:stdp-executable stdp-executable
                 #:transformed-image-data transformed-image-data
                 #:step step
                 #:seed seed
                 #:step-test step-test))
