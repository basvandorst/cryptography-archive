
;; Functions for calling RIPEM from emacs mail.
;; Version 1.1
;; This file is public domain courtesy of RSA Data Security. Inc.

;; To install, add the expresion (load "/home/local/ripem/util/emacs/ripem.el") 
;;  to the file ~/.emacs or to /home/local/emacs/lisp/default, where
;;  /home/local/ripem/util/emacs is the directory where this file is, and
;;  /home/local/emacs is the directory where emacs is installed.

;; These commands assume that the ripem executable is on the path.
;; These use the default public key file /usr/local/etc/rpubkeys and the
;;   default private key file ~/.ripemprv .
;; Be sure to generate your keys first.  Use ESC-x ripem-generate.
;; The commands ESC-x ripem-sign and ripem-encrypt use the -h option when
;;   preparing the message, which means that the header with the To: fields,
;;   etc. should be at the top of the buffer, followed by a blank line or
;;   a line with "--text follows this line--".
;; Use ESC-x ripem-receive to decipher the RIPEM message in the buffer.
;; Use ESC-x ripem-list-users to display the user names in the public key file.
;; For more info, type C-h f and then the name of the function.

(defun ripem-sign ()
  "Use this to replace the contents of your mail message buffer with a signed
  mail message, preserving the mail header.
You must enter your private key password.  Then simply send the message
  as usual.
If there is any error, type ESC-x undo"
  (interactive)
  (open-after-mail-delimiter)
  (shell-command-on-region
   (point-min) (point-max)
   (format "ripem -e -m mic-only -h p -k \"%s\"" 
           (read-string-no-echo "Enter password to private key: "))
   1)
  (goto-char (point-min)))

(defun ripem-encrypt ()
  "Use this to replace the contents of your mail message buffer with an encrypted
  mail message, preserving the mail header and encrypting for each of the
  recipients in the To: and cc: fields.  These must be the full user names as
  they appear in the public key file, such as alice@chirality.rsa.com.
  See ripem-list-users.
You must enter your private key password and a string of random bytes (for
  generating a fresh encryption key).   Then simply send the message as usual.
If there is any error, type ESC-x undo"
  (interactive)
  (open-after-mail-delimiter)
  (shell-command-on-region
   (point-min) (point-max)
   (format "ripem -e -m encrypted -T am -h pr -k \"%s\" -R sc -C \"%s\""
            (read-string-no-echo "Enter password to private key: ")
	    ;; get 64 bits of random data, assuming 4 usable bits per char
            (read-n-chars-no-echo
	     "Enter %d random characters for encryption key" 16))
   1)
  (goto-char (point-min)))

(defun open-after-mail-delimiter ()
  "Ensure that there is an open line after the --text follows this line--
  delimiter in the mail buffer so that RIPEM will find the body."
  (goto-char (point-min))
  (and (re-search-forward "^--text follows this line--
." (point-max) t)
       ;; found a char on the beginning of the line which should be blank,
       ;;   so back up and insert a newline before it
       (goto-char (1- (point)))
       (insert "\n")))

(defun ripem-list-users ()
  "Show all the user names which can be used as recipients in ripem-encrypt or
  for verifying signatures in ripem-receive."
  (interactive)
  (shell-command "grep User: /usr/local/etc/rpubkeys"))

(defun ripem-receive ()
  "Use this to decode the privacy enhanced message in the current buffer,
  placing the plain text in a separate window.
If the message is encrypted, you must enter your private key password."
  (interactive)
  (save-excursion
    ;; Go to beginning of buffer to search for ENCRYPTED string
    (goto-char (point-min))
    (if (re-search-forward "2001,ENCRYPTED" (point-max) t)
        ;; Found ENCRYPTED string, so ask user for password
        (shell-command-on-region
         (point-min) (point-max) 
         (format "ripem -d -k \"%s\""
                 (read-string-no-echo "Enter password for decrypting: ")))
      ;; else receive as non-encrypted message
      (shell-command-on-region (point-min) (point-max) "ripem -d"))))

(defun ripem-generate ()
  "Generates key files ~/.ripemprv and ~/.ripempub using the default user name.
You must enter your private key password and a long string of random bytes
  (for generating the keys).
Your username will have your full host name, such as alice@chirality.rsa.com.
  (If you want a different username, set up your RIPEM_USER_NAME environment
   variable and then generate.)
When done, append ~/.ripempub to /usr/local/etc/rpubkeys.  After this,
  ~/.ripempub is no longer used directly, but you can send it to others.
Also, archive ~/.ripemprv and don't forget your password!"
  (interactive)
  (let ((password (read-string-no-echo
		   "Enter password for private key (and don't forget it): " t))
	;; get 512 bits of random data, assuming 4 usable bits per char
	(random-chars (read-n-chars-no-echo
		       "Enter %d random characters for generating keys" 128)))
    (message "Generating ...")
    (shell-command
     (format "ripem -g -S ~/.ripemprv -P ~/.ripempub -k \"%s\" -R sc -C \"%s\""
           password random-chars)))
  (message "Done. Archive ~/.ripemprv and append ~/.ripempub to /usr/local/etc/rpubkeys"))

(defun read-string-no-echo (prompt &optional confirm)
  "Read a string from the minibuffer, prompting with PROMPT.
Optional second argument CONFIRM non-nil means that the user will be asked
  to type the string a second time for confirmation and if there is a
  mismatch, the process is repeated.

Line editing keys are:
  C-h, DEL      rubout
  C-u, C-x      line kill
  C-q, C-v      literal next"
  (catch 'return-value
    (save-excursion
      (let ((input-buffer (get-buffer-create " *password*"))
            (cursor-in-echo-area t)
            (echo-keystrokes 0)
            char string help-form done kill-ring)
        (set-buffer input-buffer)
        (unwind-protect
            (while t
              (erase-buffer)
              (message prompt)
              (while (not (memq (setq char (read-char)) '(?\C-m ?\C-j)))
                (if (setq form
                          (cdr
                           (assq char
                                 '((?\C-h . (delete-char -1))
                                   (?\C-? . (delete-char -1))
                                   (?\C-u . (delete-region 1 (point)))
                                   (?\C-x . (delete-region 1 (point)))
                                   (?\C-q . (quoted-insert 1))
                                   (?\C-v . (quoted-insert 1))))))
                    (condition-case error-data
                        (eval form)
                      (error t))
                  (insert char))
                (message prompt))
              (cond ((and confirm string)
                     (cond ((not (string= string (buffer-string)))
                            (message
                             (concat prompt "[Mismatch... try again.]"))
                            (ding)
                            (sit-for 2)
                            (setq string nil))
			   ((string= string "")
                            (message
                             (concat prompt
				     "[String cannot be empty... try again.]"))
                            (ding)
                            (sit-for 2)
                            (setq string nil))
                           (t (throw 'return-value string))))
                    (confirm
                     (setq string (buffer-string))
                     (message (concat prompt "[Retype to confirm...]"))
                     (sit-for 2))
                    (t (throw 'return-value (buffer-string)))))
          (message (concat prompt "[Thank you!]"))
          (set-buffer-modified-p nil)
          (kill-buffer input-buffer))))))

(defun read-n-chars-no-echo (prompt n)
  "Display prompt and return a string of n characters, without echoing.
If prompt has %d in it, the countdown from n is printed there.
After n chars have been read, this still adds them to the string, and
  this waits for the user to hit enter."
  (let ((echo-keystrokes 0)
	(string "")
	(count n)
	char)
    (while (> count 0)
      (message prompt count)
      (setq string (concat string (list (read-char))))
      (setq count (1- count)))
    (while (progn
	     (message "Hit ENTER when done")
	     (not (or (= (setq char (read-char)) 13) (= char 10))))
      (setq string (concat string (list char))))
    (message "Hit ENTER when done [Thank you!]")    
    string))

