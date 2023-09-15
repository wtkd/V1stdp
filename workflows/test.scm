(define test
  (command #:inputs
           (stdp-executable #:type File)
           (transformed-image-data #:type File)
           (step #:type int)
           (seed #:type int)
           (lateral-weight-dat #:type File)
           (feedforward-weight-dat #:type File)
           #:run stdp-executable "test" "--save-directory" "." "--input-file" transformed-image-data "--step" step "--seed" seed
           "--lateral-weight" lateral-weight-dat "--feedforward-weight" feedforward-weight-dat
           #:outputs
           (log #:type stdout)
           (lastnspikes #:type File
                        #:binding '((glob . "lastnspikes_test.txt")))
           (resps-test #:type File
                       #:binding '((glob . "resps_test.txt")))
           (lastnv-test #:type File
                        #:binding '((glob . "lastnv_test.txt")))))

(workflow ((stdp-executable #:type File
                            #:default '((class . "File")
                                        (location . "../build/src/main/stdp")))
           (transformed-image-data #:type File
                                   #:default '((class . "File")
                                               (location . "../patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat")))
           (step #:type int #:default 1000)
           (seed #:type int #:default 0)
           (lateral-weight-dat #:type File)
           (feedforward-weight-dat #:type File))
          (test #:stdp-executable stdp-executable
                #:transformed-image-data transformed-image-data
                #:step step
                #:seed seed
                #:lateral-weight-dat lateral-weight-dat
                #:feedforward-weight-dat feedforward-weight-dat))
