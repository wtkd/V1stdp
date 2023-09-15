(define learn
  (cwl-workflow "learn.cwl"))

(define test
  (cwl-workflow "test.cwl"))

(workflow ((stdp-executable #:type File)
           (transformed-image-data #:type File)
           (seed #:type int #:default 0)
           (step-learn #:type int #:default 1000000)
           (step-test #:type int #:default 1000))
          (pipe (learn #:stdp-executable stdp-executable
                       #:transformed-image-data transformed-image-data
                       #:seed seed
                       #:step step-learn)
                (test #:stdp-executable stdp-executable
                      #:transformed-image-data transformed-image-data
                      #:seed seed
                      #:step step-test
                      #:lateral-weight-dat lateral-weight-dat
                      #:feedforward-weight-dat feedforward-weight-dat)))
