; Routines to call PGP as a process under emacs' control. Calls are
; provided to encrypt, encrypt and sign, sign, sign in cleartext, and
; decrypt.

; If PGP fails, the 'PGP Output ' buffer is made current. This will
; hold any output from PGP. Normally the user will not see this buffer.

; PGP is run as a filter, so that no files are created on disk.

; The passphrase is generally sent via the standard input stream (to
; avoid putting it on the command line). This requires that the
; PGPPASSFD environment variable is set to "0". Under UNIX, emacs can
; set the environment variables of processes that it starts up. Under
; some other systems (amigados for instance) this doesn't work.
; Therefore there is a variable, 'pgp-env-works' which should be set
; to 't' (true) if emacs *can* set the environment variables, and
; 'nil' (false) otherwise. In the latter case, the user itself must
; set the environment variable PGPPASSFD to "0" - the routines here
; will return an error message if this is not done. The default value
; of 'pgp-env-works' is 't'. No command is provided for setting this
; variable - it should be set in the .emacs startup file.

; If PGPPASSFD is set (and PGPPASS is *not* set), a passphrase must be
; sent with every file, even if it is not really needed (eg when using
; the decrypt mode to check signatures). In such cases, a carriage
; return may be entered at the passphrase prompt.

; A default passphrase may be set (for the duration of the emacs
; session) with the 'pgp-set-pass' command. The 'pgp-clear-pass'
; command clears it again. This may be a security hole - a core dump
; of emacs would presumably contain the passphrase.

; If the PGPPASS environment variable is set, then no passphrase will
; be sent or prompted for.

; The default user for signatures may be set using the 'pgp-set-user'
; command.The 'pgp-clear-user' clears it again. If set, the '-u'
; option is used to send this value to PGP when signing.

; The name of the PGP executable may be set with the
; 'pgp-set-progname' command (default is just 'pgp' - ie it is assumed
; to be in the current path).

; When encrypting, the user will be prompted for the name of the
; recipient.  A list of names may be given, separated by white space
; (and therefore no recipient name may contain white space
; characters).

; Three variables control how fast data is sent to the PGP process.
; If 'pgp-send-slowly' is nil, then the data is sent in one chunk. If
; it is set to true, the data is sent in chunks whose size is
; 'pgp-block-size' bytes, with a delay between each block. The delay
; is determined by the delay counter 'pgp-send-delay' (the delay is in
; arbitrary units, and depends on how fast you system runs).  I don't
; like this (it feels like a kludge) but it is there because the UNIX
; and/or emacs system that I run on doesn't work if data is sent to a
; process too quickly.


;;=====================
;; define some global variables
;;=====================

(defvar pgp-key-phrase nil "Passphrase for PGP (default is nil)")

(defvar pgp-progname "pgp" "*Name of PGP executable (default is pgp)")

(defvar pgp-user nil "*Name of user for signature purposes (default is
nil)")

(defvar pgp-begin-marker "-----BEGIN PGP")

(defvar pgp-env-modified nil "Set true if the 'PGPPASSFP' environment
variable has been set up")

(defvar pgp-env-works t "*True if modifying 'process-environment' has any
effect on processes started by emacs (must be 'nil' for amiga)")

(defvar pgp-proc-var nil)

(defvar pgp-src-buffer nil "buffer holding original data")

(defvar pgp-res-buffer nil "Buffer to which PGP output goes")

(defvar pgp-src-start nil "Location in source buffer where original data
starts")

(defvar pgp-src-end nil "Location in source buffer where original data
ends")

(defvar pgp-filter-results nil "Set to true if PGP output will have to
be stripped from the output data")

(defvar pgp-send-slowly nil "*Send the data tp PGP in small chunks, with a delay between each chunk")

(defvar pgp-send-delay 0 "*Delay counter used when 'pgp-send-slowly' is true")

(defvar pgp-block-size 3 "*Size of chunks sent to PGP when 'pgp-send-slowly' is true")

;;=====================
;; routines to check/set up the 'PGPPASSFD' environment variable
;;=====================

(defun stripstrlist (l str)
  "Strip from list-of-strings L any string which matches STR."
  (cond (l (cond ((string-match str (car l))
		  (stripstrlist (cdr l) str))
		 (t (cons (car l) (stripstrlist (cdr l) str)))))))

(defun pgp-modify-env ()
  "set up the PGPPASSFD environment variable so that PGP reads the
passphrase from the input stream"
  (let ((tmaxdep max-lisp-eval-depth))
;; allow for many environment variables
    (setq max-lisp-eval-depth 400)
    (setq process-environment
	  (cons "PGPPASSFD=0"
		(stripstrlist process-environment "^PGPPASSFD=")))
;; go back to the original value
    (setq max-lisp-eval-depth tmaxdep)
;; we don't want to do this over and over again
    (setq pgp-env-modified t)
    )
;; return 'true' to indicate that the environment is ok now
  t
)

(defun pgp-set-passfd-envar ()
  "make sure that the PGPPASSFD environment variable is set up"
  (if pgp-env-works
;; modify the environment if we haven't already; return 'true' if we have
      (if (not pgp-env-modified) (pgp-modify-env) t)
;; otherwise get the variable from outside and check its value is ok
;; (ie "0") returning false if it isn't
    (if (string-equal "0" (getenv "pgppassfd")) t nil))
)

;;=====================
;; routines to set up some user-definable variables
;;=====================

(defun pgp-set-progname (pname)
  "set the name of the executable that is called; a path is not required
if the executable is in the search path"
  (interactive "sEnter program name: ")
  (setq pgp-progname pname))

(defun pgp-set-user (uname)
  "set the name of the user when a signature is requested (ie
a string to be used with PGP's '-u' option)"
  (interactive "sEnter user name: ")
  (setq pgp-user uname))

(defun pgp-clear-user ()
  "clear the 'pgp-user' variable; no '-u' option will be passed to
PGP when signatures are requested"
  (interactive)
  (setq pgp-user nil))

(defun pgp-set-pass ()
  "set the passphrase, so that the routines do not prompt for it
on every call to PGP. Not recommended"
  (interactive)
  (let ((passphrase (pgp-read-pass )))
    (setq pgp-key-phrase passphrase)))

(defun pgp-clear-pass ()
  "Clear the variable holding the passphrase"
  (interactive)
  (setq pgp-key-phrase nil))

;;=====================
;; routines to read password with echo turned off. Taken from code
;; by ratinox@meceng.coe.northeastern.edu (Stainless Steel Rat), who
;; got it from the ange-ftp.el code
;;=====================

(defun pgp-read-pass (&optional default)
  "Read a password from the user. Echos a " " for each character typed.
End with RET, LFD, or ESC. DEL or C-h rubs out.  ^U kills line.
Optional DEFAULT is password to start with."
  (let ((pass (if default default ""))
	(c 0)
	(echo-keystrokes 0)
	(cursor-in-echo-area t))
    (while (and (/= c ?\r) (/= c ?\n) (/= c ?\e))
      (message "%s%s"
	       "Enter pass phrase: "
	       (make-string (length pass) ? ))
      (setq c (read-char))
      (if (= c ?\C-u)
	  (setq pass "")
	(if (and (/= c ?\b) (/= c ?\177))
	    (setq pass (concat pass (char-to-string c)))
	  (if (> (length pass) 0)
	      (setq pass (substring pass 0 -1))))))
    (pgp-repaint-minibuffer)
    (substring pass 0 -1)))

(defun pgp-repaint-minibuffer ()
  "Gross hack to set minibuf_message = 0, so that the contents of the
minibuffer will show."
  (if (eq (selected-window) (minibuffer-window))
      (if (fboundp 'allocate-event)
	  ;; lemacs
	  (let ((unread-command-event (character-to-event ?\C-m
							  (allocate-event)))
		(enable-recursive-minibuffers t))
	    (read-from-minibuffer "" nil pgp-tmp-keymap nil))
	;; v18 GNU Emacs
	(let ((unread-command-char ?\C-m)
	      (enable-recursive-minibuffers t))
	  (read-from-minibuffer "" nil pgp-tmp-keymap nil)))))

;;=====================
;; routines to turn a string into a list of substrings. Substrings are
;; strings of non-space characters delimited by any number of spaces.
;;=====================

(defun string-to-list (string &optional seed)
  (let ((hstr (string-get-fsub string)))
    (if (string= "" hstr) seed
      (cons hstr (string-to-list (string-get-rsub string) seed)))))

(defun string-get-rsub (string)
  (let ((cpos (string-match "[ ]*[^ ]+[ ]" string)))
    (if (eq cpos nil) "" (substring string (match-end 0)))))

(defun string-get-fsub (string)
  (let ((cpos (string-match "[^ ]" string)))
	(if (eq cpos nil) ""
	  (substring string cpos (string-match "[ ]" string cpos)))))


;;=====================
;; code for the PGP process's sentinel - called whenever the process changes
;; state..
;;=====================

(defun pgp-replace-buffer ()
  "replaces the contents of the original buffer/region with the PGP output"
  (switch-to-buffer pgp-src-buffer)
  (let ((filter pgp-filter-results))
    (kill-region pgp-src-start pgp-src-end)
    (switch-to-buffer pgp-res-buffer)
    (goto-char (point-min))
    (if filter (search-forward pgp-begin-marker))
    (beginning-of-line)
    (kill-region (point) (point-max))
    (switch-to-buffer pgp-src-buffer)
    (kill-buffer pgp-res-buffer)
    (yank)))

(defun pgp-display-res (result)
  "switch to the PGP results buffer, and insert 'result' at the end"
  (switch-to-buffer pgp-res-buffer)
  (goto-char (point-max))
  (insert result))

(defun pgp-sentinel (sproc string)
  "If the string received is 'finished\n', then replace the original text
by the PGP output buffer; otherwise switch to the output buffer and
insert the string at the end"
;;
;; handle normal exit
  (if (string-equal string "finished\n")
      (pgp-replace-buffer)
;;
;; handle exit code 1 (returned when decrypting, for some reason !)
    (if (string-equal string "exited abnormally with code 1\n")
	(pgp-replace-buffer)
    (pgp-display-res string))))

;;=====================
;; the main work routines
;;=====================

(defun pgp-do-work
  (bstart bend reqpass filter pgp-arg)
  "Set up the env variable (if need be) and call the main work routine"
  (if (pgp-set-passfd-envar)
      (pgp-do-work-1 bstart bend reqpass filter pgp-arg)
    (error "The environment variable 'PGPPASSFD' must be set to '0'")))


(defun pgp-do-work-1 (bstart bend reqpass filter pgp-arg)
  "The main work routine - sets up an output buffer, starts up PGP,
sets up a sentinel for the PGP process, and feeds PGP with the
passphrase and data"
  (setq pgp-res-buffer (get-buffer-create "PGP Output"))
  (setq pgp-src-buffer (current-buffer))
  (setq pgp-src-start bstart)
  (setq pgp-src-end bend)
  (setq pgp-filter-results filter)

  (switch-to-buffer pgp-res-buffer)
  (erase-buffer)
  (switch-to-buffer pgp-src-buffer)
;;
;; Call the 'start-process' function with the relevant arguments (I use
;; 'apply' since I have a variable number of args, but don't want to
;; pass any null args. Is there a better way of doing this ?).
;;
;; We use the 'verbose' flag to get rid of junk from the output (don't
;; know how to separate 'stderr' and 'stdout') and batchmode so that an
;; incorrect passphrase doesn't result in a prompt from PGP.
;;
  (setq pgp-proc-var
	(apply 'start-process "procpgp" pgp-res-buffer pgp-progname
	       "+batchmode" "+verbose=0" pgp-arg))

  (set-process-sentinel pgp-proc-var 'pgp-sentinel)

;; If the PGPASSFP env-var is set, and the PGPPASS env-var *isn't*
;; set, PGP will *always* read a passphrase from the input stream,
;; even if it doesn't need one. So here we send nothing (if PGPPASS is
;; set), or we just send some junk if a passphrase isn't really
;; needed, or we send the default passphrase, if set, or we prompt the
;; user for one and send that. If anything was sent, we follow up with
;; a carriage return.

  (if (eq (getenv "PGPPASS") nil)
      (if reqpass
	  (if(eq pgp-key-phrase nil)
	      (process-send-string pgp-proc-var (pgp-read-pass))
	    (process-send-string pgp-proc-var pgp-key-phrase))
	(process-send-string pgp-proc-var
			     "**If you see this, an error has occured in the emacs routines or in setting up**"
			     )
	)
    )

  (if (eq (getenv "PGPPASS") nil)
      (process-send-string pgp-proc-var "\n"))

  ;; Depending on the value of 'pgp-send-slowly', we either send the
  ;; data line by line, or we send it all in one go. We have to send
  ;; line by line on systems where 'process-send-region' sends the
  ;; data too quickly, causing an error (this happens at least on UNIX
  ;; SunOS4.1 on a SPARC, emacs version 18.57.54, but not on my amiga
  ;; version. I've no idea why).

  (if (eq t pgp-send-slowly)
      (pgp-send-in-chunks pgp-proc-var bstart bend)
    (process-send-region pgp-proc-var bstart bend)
    )

  ;; finally send an end-of-file character
  (process-send-eof pgp-proc-var)
  )

(defun pgp-send-in-chunks (proc start end)
  (let ((chars-left (- end start)))
    (let ((waitc nil))
      (let ((lstart nil))
	(save-excursion
	  (goto-char start)
	  (while (> chars-left 0)
	    (setq lstart (point))
	    (goto-char (+ lstart pgp-block-size))
	    (process-send-string proc (buffer-substring lstart (point)))
	    (setq chars-left (- chars-left pgp-block-size))
	    (setq waitc 0)
	    (while (not (= pgp-send-delay waitc)) (setq waitc (+ waitc 1)))
	    )
	  )
	)
      )
    )
  )


;;=====================
;; these are the main encryption routines, most of which come in two
;; flavours - work on region, and work on the whole buffer
;;=====================

(defun pgp-encrypt-region (min max to)
"Use PGP to encrypt the region. It prompts for the name of a recipient.
Ascii armour is used for the output"
  (interactive "r\nsRecipient name: ")
  (pgp-do-work min max nil nil (cons "-feat" (string-to-list to) )))

(defun pgp-encrypt-buffer (to)
"Use PGP to encrypt the buffer. It prompts for the name of a recipient.
Ascii armour is used for the output"
  (interactive "sRecipient name: ")
  (pgp-encrypt-region (point-min) (point-max) to))



(defun pgp-decrypt-region (min max)
  "Use PGP to decrypt the region. It prompts for the passphrase if
none is set."
  (interactive "r")
  (pgp-do-work min max t nil (list "-f")))

(defun pgp-decrypt-buffer ()
  "Use PGP to decrypt the buffer. It prompts for the passphrase if
none is set."
  (interactive)
  (pgp-decrypt-region (point-min) (point-max)))



(defun pgp-sign-region (min max)
  "Use PGP to sign the region. It prompts for the passphrase if none is
set, and for the name of the signer if none is set. Ascii armour is used
for the output"
  (interactive "r")
  (if (eq nil pgp-user)
      (pgp-do-work min max t t (list "-fsat"))
    (pgp-do-work min max t t (list "-fsa" (concat "-u" pgp-user)))
    ))

(defun pgp-sign-buffer ()
  "Use PGP to sign the buffer. It prompts for the passphrase if none is
set, and for the name of the signer if none is set. Ascii armour is used
for the output"
  (interactive)
  (pgp-sign-region (point-min) (point-max)))



(defun pgp-clsign-region (min max)
  "Use PGP to sign the region with cleartext. It prompts for the passphrase
if none is set, and for the name of the signer if none is set.
Ascii armour is used for the output"
  (interactive "r")
  (if (eq nil pgp-user)
      (pgp-do-work min max t t (list "-fsat" "+clearsig=on"))
    (pgp-do-work min max t t (list "-fsat" "+clearsig=on" (concat "-u" pgp-user)))
    ))

(defun pgp-clsign-buffer ()
  "Use PGP to sign the buffer with cleartext. It prompts for the passphrase
if none is set, and for the name of the signer if none is set.
Ascii armour is used for the output"
  (interactive)
  (pgp-clsign-region (point-min) (point-max)))



(defun pgp-sign-encrypt-region (min max to)
  "Use PGP to sign and encrypt the region. It prompts for the passphrase
if none is set, and for the name of the signer if none is set.
Ascii armour is used for the output"
   (interactive "r\nsRecipient name: ")
   (pgp-do-work min max t t (cons "-fseat" (string-to-list (concat to " -u" pgp-user)))))

(defun pgp-sign-encrypt-buffer (to)
  "Use PGP to sign and encrypt the buffer. It prompts for the passphrase
if none is set, and for the name of the signer if none is set.
Ascii armour is used for the output"
  (interactive "sRecipient name: ")
  (pgp-sign-encrypt-region (point-min) (point-max) to))

(global-set-key "\C-cpeb" 'pgp-encrypt-buffer)
(global-set-key "\C-cper" 'pgp-encrypt-region)
(global-set-key "\C-cpdb" 'pgp-decrypt-buffer)
(global-set-key "\C-cpdr" 'pgp-decrypt-region)
(global-set-key "\C-cpcb" 'pgp-clsign-buffer)
(global-set-key "\C-cpcr" 'pgp-clsign-region)
(global-set-key "\C-cpsb" 'pgp-sign-buffer)
(global-set-key "\C-cpsr" 'pgp-sign-region)
(global-set-key "\C-cpab" 'pgp-sign-encrypt-buffer)
(global-set-key "\C-cpar" 'pgp-sign-encrypt-region)
