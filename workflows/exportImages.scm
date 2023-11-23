(define genarate-text-images
  (command #:inputs
           (stdp-executable #:type File)
           (transformed-image-data #:type File)
           (edge-length #:type int)
           (on-image-directory-name #:type string)
           (off-image-directory-name #:type string)
           (text-image-directory-name #:type string)
           #:run
           stdp-executable "tool" "image" "export"
           transformed-image-data
           "--edge-length" edge-length
           "--on-image-directory" on-image-directory-name
           "--off-image-directory" off-image-directory-name
           "--all-each" text-image-directory-name
           #:outputs
           (on-images-text-directory
            #:type Directory
            #:binding ((glob . "$(inputs[\"on-image-directory-name\"])")))
           (off-images-text-directory
            #:type Directory
            #:binding ((glob . "$(inputs[\"off-image-directory-name\"])")))
           (text-images-directory
            #:type Directory
            #:binding ((glob . "$(inputs[\"text-image-directory-name\"])")))))

(define generate-svg-images
  (command #:inputs
           (stdp-executable #:type File)
           (total-image-number #:type int)
           (on-images-text #:type Directory)
           (off-images-text #:type Directory)
           (output-directory #:type string)
           (gnuplot-script #:type File
                           #:default '((class . "File")
                                       (location . "../script/onOffImages.gnuplot")))
           #:run
           "gnuplot"
           "-e" "n=$(inputs[\"total-image-number\"])"
           "-e" "onInputDirectory='$(inputs[\"on-images-text\"].path)'"
           "-e" "offInputDirectory='$(inputs[\"off-images-text\"].path)'"
           "-e" "outputDirectory='$(inputs[\"output-directory\"])'"
           "-e" "stdpExecutable='$(inputs[\"stdp-executable\"].path)'"
           gnuplot-script
           #:outputs
           (svg-images-directory
            #:type Directory
            ;; #:secondaryFiles
            ;; '((class . "File")
            ;;   (location . "$(inputs[\"output-directory\"].location)"))
            #:binding ((glob . "$(inputs[\"output-directory\"])"))
            )))

(workflow ((stdp-executable #:type File
                            #:default '((class . "File")
                                        (location . "../build/src/main/stdp")))
           (transformed-image-data #:type File
                                   #:default '((class . "File")
                                               (location . "../patchesCenteredScaledBySumTo126ImageNetONOFFRotatedNewInt8.bin.dat")))
           (total-image-number #:type int #:default 109999)
           (edge-length #:type int #:default 17))
          (pipe
           (genarate-text-images
            #:stdp-executable stdp-executable
            #:transformed-image-data transformed-image-data
            #:edge-length edge-length
            #:on-image-directory-name "onImagesText"
            #:off-image-directory-name "offImagesText"
            #:text-image-directory-name "imagesText")
           (tee
            (rename #:text-images-directory text-images-directory
                    #:on-images-directory on-images-text-directory
                    #:off-images-directory off-images-text-directory)
            (generate-svg-images
             #:stdp-executable stdp-executable
             #:total-image-number total-image-number
             #:on-images-text on-images-text-directory
             #:off-images-text off-images-text-directory
             #:output-directory "svgInputImages"))))
