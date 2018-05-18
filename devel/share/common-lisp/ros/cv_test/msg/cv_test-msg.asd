
(cl:in-package :asdf)

(defsystem "cv_test-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "MsgControl" :depends-on ("_package_MsgControl"))
    (:file "_package_MsgControl" :depends-on ("_package"))
  ))