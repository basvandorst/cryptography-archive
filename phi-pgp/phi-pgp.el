;;; --------------- COPYRIGHT ------------------------
;;;| AIT - Computer Science Program - Thailand        |
;;;|   see COPYRIGHT notice                           |
;;;--------------------------------------------------

;;; --------------- PRODUCT IDENTIFICATION -----------
;;;| Product              : phi-pgp                   |
;;;| Date                 : November 1993             |
;;;| Last update          : 22 Nov 93                 |
;;;|                      : Aug 94 for version 2.6ui  |
;;;| Conceived by         : O. Nicole                 |
;;;| Purpose              : GNU Emacs interface for PGP
;;;--------------------------------------------------

;;; --------------- HISTORY ------------------------
;;;|%c%--------------------------------------------------

;;; --------------- CONSTANTS ----------------------

;;; This are strings genearted by output of PGP

(setq pgp-strconst-enter-pass "\nEnter pass phrase: ")
					; Prompt requesting for the passphrase
(setq pgp-strconst-bad-pass "\nError:  Bad pass phrase\\.\n")
					; Message for s bad pass phrase.
(setq pgp-strconst-pass-key "^Key for user ID \".*\"\n")
					; Message specifying the key which
					; pass phgrase is about to be read
(setq pgp-strconst-key-id "^Key for user ID:")
					; Message specifying the key ID used 
					; for secret of public key
(setq pgp-strconst-signed-mess
      "^File has signature.  Public key is required to check signature.")
					; Message issued when a signed
					; message is decrypted.
(setq pgp-strconst-good-sign "^Good signature from user \".*\"\\.$")
					; Message issued when a valid 
					; signature is encountred.
(setq pgp-strconst-bad-sign "^WARNING: Can't find the right public key-- can't check signature integrity.$")
					; Message issued when there is no
					; public key corresponding to the
					; signature of the message.
(setq pgp-strconst-packet-sep
      "\n?\nLooking for next packet in '.*'\\.\\.\\.$")
					; Message issued when there are 
					; several pgpcoded packet in a
					; same file.
(setq pgp-strconst-good-pass
      "^Pass phrase is good.  Just a moment\\.\\.\\.\\.\\.\\.")
					; Message issued just before the
					; message (no \\n) when the pass
					; is good,
(setq pgp-strconst-recur-sep "Trying a recursive call to pgp...\n")
					; Message added when a automatic
					; recursive call to pgp is generated.
(setq pgp-strconst-crypted-mess
      "^File is encrypted.  Secret key is required to read it. $")
					; Message issued when decrypt a text.
(setq pgp-strconst-double-bad-pass
      "^You do not have the secret key needed to decrypt this file.$")
					; Message send when the pass phrase
					; have been entered incorectely
					; twice while decrypt.
(setq pgp-strconst-bad-ascii
      "^Error: Transport armor stripping failed for file ")
					; Message issued when the packet has
					; been altered during the mailransport
(setq pgp-strconst-begin-ro
  "^This message is marked \"For your eyes only\".  Display now (Y/n)\\? y?$")
					; Promp message displayed when a
					; view-only text is decrypted.
(setq pgp-strconst-end-ro "^Done\\.\\.\\.hit any key\\( +\\)?$")
					; Prompt message at the end of a
					; 'view-only' section.
(setq pgp-strconst-more-ro "-- Hit space for next screen, Enter for new line, 'Q' to quit --\\( +\\)?")
					; Prompt message in the middle of a
					; 'view-only' section
(setq pgp-strconst-add-keyring
      "^Do you want to add this keyfile to keyring '.*' (y/N)\\? $")
					; Prompt for adding a key to a keyring
(setq pgp-strconst-key-mess
      "^File contains key(s)\\.  Contents follow\\.\\.\\.\nKey ring: '.*'\n")
					; Message when the decrypted
					; mail contains key.
(setq pgp-strconst-end-key-mess "^[0-9]+ key(s) examined\\.$")
					; Message at the end of a
					; decrypted mail containing keys.
(setq pgp-strconst-no-key-found "^No keys found in '.*'\\.$")
					; Message when no valid key is
					; found while adding key.

;;; --------------- MACROS -------------------------

;;; --------------- STATIC VARIABLES ---------------

(defvar pgp-list-keys nil)
					;Lists of public and secret keys
					; extracted from pgp-keyrings
;;;(setq pgp-exit-status nil)
					; Exit status of the latest pgp
					; asynchromous process
;;;(setq pgp-buffer-search-point nil)
					; Last character of *PGP process*
					; buffer already examinated
(setq pgp-list-completion nil)
					; Define the list of pgp possible
					; keys used for a key read
(setq pgp-selected-seckey nil)
					;The secret key currently in use.
					; It is set to the user define
					; pgp-default-seckey or to the first
					; key of secring. It is changed using
					; thegp-set-seckey function
(defvar pgp-list-secring-set nil
  "Set non-nil means a list of secret keys have been extracted form secring.")
(defvar pgp-list-pubring-set nil
  "Set non-nil means a list of public keys have been extracted form pubring.")
(defvar pgp-program "pgp"
  "Define the name of the local pgp program, if not in the default PATH, the
name should be absolute.")

;;; --------------- USER VARIABLES -----------------

(defvar pgp-default-seckey nil
  "*User define default secret key to use. If not define, will choose the last
added key of secring.")
(defvar pgp-delete-pass nil
  "*Set non-nil means always delete the pass phrase. Keeping a pass phrase 
associated to a secret key during an Emacs session can be a security hole, 
if the pass phrase is in memory of Emacs when it crashes, it will appear 
in the core file.")
(defvar pgp-self-crypt nil
  "*Set to non-nil means encrypt with user's own key in addition of recipients
keys.")
(defvar pgp-auto-add-pubkey nil
  "*Set to non-nil, try to automatically add the user's public key inside
an encrypted message. No meaning for the signed message, the view-only 
messages of the partially encrypted ones. The key is added at the very
end of the message.")

;;; --------------- END PRODUCT IDENTIFICATION -----

;;; --------------- Function identification ----------
;;;| Function name         : pgp-read-passphrase      |
;;;| Purpose               : Read a pass phrase from the minibuffer
;;;| Calling parameters    :                          |
;;;|  prompt: the string that it is prompt at screen  |
;;;| Return value          : the pass phrase or nil   |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-get-passphrase        |
;;;-------------------------------------------------- 

;;; --------------- Function definition ------------

;;;Stolen from rat-pgp.el
(defun pgp-read-passphrase (prompt)
  "Read a pass phrase form the minibuffer. 
Echos a '.' for each character typed. 
RET   end the pass phrase
LFD   end the pass phrase
DEL   delete characters backward
C-k   clear the pass phrase
C-g   abort"

;;; --------------- End function identification ----

  (let ((inhibit-quit t)
					; no quit alowed during this time
	(pass "")
	(c 0)
	(win-larg (window-width (minibuffer-window)))
					; get the width of current minibuffer
	(echo-keystrokes 0)
	(cursor-in-echo-area t))
    (discard-input)
    (while (and (/= c ?\r) (/= c ?\n) (/= c ?\007))
	(let ((mess (concat prompt (make-string (length pass) ?.))))
	  (if (> (length mess) win-larg)
					; horizontal scroll of minibuffer
	      (message (substring mess (- win-larg)))
	    (message mess)
	    ))
      (setq c (read-char))
      (if (= c ?\C-k)
					; erase line
	  (setq pass "")
	(if (/= c ?\177)
	    (if (and (>= c ?\040) (< c ?\177))
					; only printable characters
		(setq pass (concat pass (char-to-string c)))
					; Emacs default to beep for
					; un-authorized chararcter
	      (if (and (/= c ?\r) (/= c ?\n) (/= c ?\007))
					; RET or LFD or C-g are ok there
		  (ding)))
	  (if (> (length pass) 0)
					; DEL
	      (setq pass (substring pass 0 -1))
					; del at the beginning of buffer
	    (progn
	      (message "Beginning of buffer")
	      (ding)
	      (sit-for 2))
	    )))
      )
;;;I cannot guess why this is for
;;;   (pgp-repaint-minibuffer)
    (if (= c ?\007)
	(progn
	  (ding)
	  (message "Quit")
	  nil)
      (if (string= pass "")
	  (progn
	    (message "PGP: empty pass phrase.")
	    (ding)
	    nil)
	pass))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;Stolen from rat-pgp.el.
;;;Not activated yet. Lightly modified for Emacs v19
;;;(defun pgp-repaint-minibuffer ()
;;;  "Gross hack to set minibuf_message = 0, so that the contents of the
;;;minibuffer will show."
;;;  (if (eq (selected-window) (minibuffer-window))
;;;      (let ((unread-command-events ?\C-m)
;;;	    (enable-recursive-minibuffers t))
;;;	(read-from-minibuffer ""))))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-get-passphrase       |
;;;| Purpose               : Read the pass phrase for a given user.
;;;| Calling parameters    :                          |
;;;| Return value          : t or nil                 |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-get-seckey pgp-read-passphrase
;;;| Calling functions    : pgp-output-filter         |
;;;|  pgp-general-sign, pgp-sign-encrypt              |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-get-passphrase ()
  "Read the pass phrase for a given user."

;;; --------------- End function identification ----

  (if (pgp-get-seckey)
      (if (cdr pgp-selected-seckey)
					; pass allready exists
	  pgp-selected-seckey
					; else read a pass

	(let ((key (pgp-read-passphrase (concat
					  "Enter the pass phrase for <"
					  (car pgp-selected-seckey) ">: "))))
	  (setq pgp-selected-seckey (cons (car pgp-selected-seckey) key))
	  (if key
	    pgp-selected-seckey
	    nil)
	  ))
    nil))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-get-seckey           |
;;;| Purpose               : return the actual secret key
;;;| Calling parameters    :                          |
;;;| Return value          : the secret key or nil    |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-list-secring, pgp-check-seckey
;;;|  pgp-set-seckey                                  |
;;;| Calling functions    : pgp-get-passphrase, pgp-view-seckey,
;;;|  pgp-general-encrypt, pgp-append-pubkey, pgp-sign-encrypt
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-get-seckey ()
  "Return the actual secret key.
The key is a cons of the key and the pass-phrase if it exists."

;;; --------------- End function identification ----

  (if (not pgp-selected-seckey)
					; dont check if key already selected
      (setq pgp-selected-seckey
	    (cond ((pgp-list-secring)
		   (if pgp-default-seckey
					; check validity of pgp-default-seckey
					; return the first matchin seckey!
		       (let ((key (pgp-check-seckey
				   pgp-default-seckey
				   (cdr pgp-list-keys))))
			 (if (not key)
					; default-key not valid
			     (pgp-set-seckey)
			   key))
					; return first key of secring
		     (car (cdr pgp-list-keys))))
		  (t nil))))
  pgp-selected-seckey)

;;; --------------- Function identification ----------
;;;| Function name         : pgp-view-seckey          |
;;;| Purpose               : Display the selected secret key
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-get-seckey            |
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-view-seckey ()
  "Display the selected secret key."
  (interactive)

;;; --------------- End function identification ----

  (if (pgp-get-seckey)
      (message "PGP: selected secret key is <%s>."
	       (car pgp-selected-seckey))
    (message "PGP: no secret key can be selected.")))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-check-seckey         |
;;;| Purpose               : Return the first matching secret key from secring
;;;| Calling parameters    :                          |
;;;|  key :the key to check                           |
;;;| list: the list to check the key against          |
;;;| Return value          : (key . pass) or nil      |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : RECURSIVE                 |
;;;| Calling functions    : pgp-get-seckey pgp-set-seckey
;;;|  pgp-set-var-seckey                              |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-check-seckey (key list)
  "Return the first matching secret key of PGP secring."

;;; --------------- End function identification ----

  (if list
      (if (string-match key (car (car list)))
	  (car list)
	(pgp-check-seckey key (cdr list)))
    (progn
      (ding)
      (message
   "PGP: the pgp-default-seckey doesn't match the available keys in secring.")
      (sit-for 3)
      nil)))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-set-seckey           |
;;;| Purpose               : the selected secret key in use
;;;| Calling parameters    :                          |
;;;| Return value          : (key . pass) or nil      |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-list-secring, pgp-check-seckey
;;;|  pgp-read-recipient                              |
;;;| Calling functions    : pgp-get-seckey            |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-set-seckey ()
  "Set the selected secret key in use."
  (interactive)

;;; --------------- End function identification ----

  (cond ((pgp-list-secring)
	 (setq pgp-selected-seckey
	       (if (= (length (cdr pgp-list-keys)) 1)
					; in case only one secret key,
					; select it, no check
		   (progn
		     (message
		      "PGP: there is only one key in the secring <%s>."
		      (car (car (cdr pgp-list-keys))))
		     (sit-for 3)
		     (car (cdr pgp-list-keys)))
		 (pgp-check-seckey
					; else read a key from user and check
		  (pgp-read-recipient
		   "Enter a secret key: " (cdr pgp-list-keys) "")
		  (cdr pgp-list-keys)))))
	(t (setq pgp-selected-seckey nil))))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-set-var-seckey       |
;;;| Purpose               : Set the selected secret key in use
;;;|   depending on the key send as argument          |
;;;| Calling parameters    :                          |
;;;|  key: the new secret key to select               |
;;;| Return value          : (key . pass)             |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-list-secring, pgp-check-seckey
;;;| Calling functions    : pgp-output-filter         |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-set-var-seckey (key)
  "Set the selected secret key in use depending on the key send as argument.
The key send as argument is a valid secret key."

;;; --------------- End function identification ----

  (cond ((pgp-list-secring)
	 (if (not (string= key (car pgp-selected-seckey)))
	     (setq pgp-selected-seckey
		   (if (= (length (cdr pgp-list-keys)) 1)
					; more than one seckey or not
					; yet selected
		       (car (cdr pgp-list-keys))
		     (pgp-check-seckey key (cdr pgp-list-keys))))))
	(t (setq pgp-selected-seckey nil))))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-extract-email-recipient
;;;| Purpose               : Extract the list of recipient from a
;;;|            *mail* buffer                         |
;;;| Calling parameters    :                          |
;;;| Return value          :list of recipients or nil |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-general-encrypt, pgp-sign-encrypt
;;;| Calling functions    : pgp-extract-recipient-feild
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-extract-email-recipient ()
  "Extract the list of recipient from a *mail* buffer.
Extract recipients of every To:, CC: or BCC: mail-header feilds as 
if one request for message encrypting, the message should be made 
readable for any recipient. The *mail* buffer is considered to be
the current buffer. Return a list of strings."

;;; --------------- End function identification ----

  (let ((end-header-point)
	(recipient-list nil)
	(old-case-fold-search))
    (save-excursion
					; We will navigate in the buffer
      (setq old-case-fold-search case-fold-search)
      (setq case-fold-search nil)
      (beginning-of-line)
      (goto-char (point-min))
      (search-forward mail-header-separator)
      (setq end-header-point (point))
					; Find the end of the header,
					; will restrict further search-forward
      (setq recipient-list
	    (append
					; there is 3 reciepients feilds
					; To, CC and BCC
	     (pgp-extract-recipient-feild "^To:[ ]*" end-header-point)
					; get recipient for To: feild
	     (pgp-extract-recipient-feild "^CC:[ ]*" end-header-point)
					; get recipient for CC: feild
	     (pgp-extract-recipient-feild "^BCC:[ ]*" end-header-point)
					; get recipient for BCC: feild
	     ))
      (setq case-fold-search old-case-fold-search)
      )
    recipient-list)
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-extract-recipient-feild
;;;| Purpose               : Extract the list of recipients for a *mail*
;;;|                 buffer for a specified feild     |
;;;| Calling parameters    :                          |
;;;|  feild: the header feild to search for           |
;;;|  end-header-point: a mark to the end of mail header
;;;| Return value          : list of recipients or nil|
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-string-to-list        |
;;;| Calling functions    : pgp-extract-email-recipient
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-extract-recipient-feild (feild end-header-point)
  "Extract the list of recipients for a *mail* buffer for a specified feild."

;;; --------------- End function identification ----

  (let ((line-read)
	(beg-point)
	(recp-list nil))
    (goto-char (point-min))
    (while (re-search-forward feild end-header-point t)
					; there can be several feilds
      (setq beg-point (point))
      (re-search-forward ".*\n\\([ ]+.*\n\\)*" end-header-point t)
      (setq line-read (downcase (buffer-substring beg-point (point))))
					; Extract the current lines of header
					; even is addresses are split on
					; several lines
      (setq recp-list (append recp-list (pgp-string-to-list line-read)))
      )
    recp-list
					; return the constructed list
    ))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-string-to-list       |
;;;| Purpose               : Make a list of single word strings from a
;;;|               string containing several words    |
;;;| Calling parameters    :                          |
;;;|  entry-str: the entry string                     |
;;;| Return value          : list of words or nil     |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-extract-recipient-feild
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-string-to-list (entry-str)
  "Make a list of single word strings from a string containing several words.
The entry string contains several words that are SPACE, NEW-LINE, 
TAB and/or comma separated. The resulting list contains single words, 
the separating characters having been removed."

;;; --------------- End function identification ----

  (if entry-str
      (let ((ret-list nil)
	    (str-point 0)
	    (test))
	(if (string-match "^[\n\t\ ,]*" entry-str)
	    (setq str-point (match-end 0)))
					; skip SPACE, TAB, NEW-LINE or ','
					; at the begining of the string
	(setq test (string-match "[^\t\n\ ,]+" entry-str str-point))
					; search first word
	(while test
	  (setq ret-list
		(append ret-list
			(list (substring entry-str str-point (match-end 0)))))
					; add the current word to the list
	  (setq str-point (match-end 0))
	  (if (string-match "[\n\t\ ,]+" entry-str str-point)
	      (setq str-point (match-end 0)))
					; remove SPACE, TAB, NEW-LINE or ','
	  (setq test (string-match "[^\t\n\ ,]+" entry-str str-point))
					; search next word
	  )
	ret-list
	)
    nil)
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-create-output-buffer |
;;;| Purpose               : Create the *PGP process* buffer
;;;| Calling parameters    :                          |
;;;| Return value          : the buffer               |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-list-pubring, pgp-list-secring
;;;|  pgp-general-sign, pgp-general-encrypt, pgp-append-pubkey,
;;;|  pgp-sign-encrypt, pgp-decr-mess, pgp-add-key    |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-create-output-buffer ()
  "Create the *PGP process* buffer.
Create or clean the buffer used for working and output by pgp
asynchronous processes."

;;; --------------- End function identification ----

  (if (get-buffer "*PGP process*")
      ;; if buffer *PGP process* exists then delete it
      (save-excursion
	(set-buffer "*PGP process*")
	(setq buffer-read-only nil)
	(erase-buffer)))
  (get-buffer-create "*PGP process*")
					; Create buffer *PGP process*
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-create-temp-buffer   |
;;;| Purpose               : Create the *PGP temp* buffer
;;;| Calling parameters    :                          |
;;;| Return value          : the buffer               |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-encrypt, pgp-encrypt-view
;;;|  pgp-encrypt-region, pgp-append-pubkey, pgp-show-status
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-create-temp-buffer ()
  "Create the *PGP temp* buffer.
Create or clean the buffer used for temporary working by pgp
asynchronous processes."

;;; --------------- End function identification ----

  (if (get-buffer "*PGP temp*")
      ;; if buffer *PGP temp* exists then delete it
      (save-excursion
	(set-buffer "*PGP temp*")
	(local-set-key "\C-ca" 'pgp-add-key)
	(local-set-key "\C-cd" 'pgp-decr-mess)
	(setq buffer-read-only nil)
	(erase-buffer)))
  (get-buffer-create "*PGP temp*")
					; Create buffer *PGP temp*
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-wait-process-exit    |
;;;| Purpose               : Wait for the termination of a pgg
;;;|                         asychronous process      |
;;;| Calling parameters    :                          |
;;;|  proc: the process to wait for                   |
;;;| Return value          : exit code of the process |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-list-pubring, pgp-list-secring
;;;|  pgp-general-sign, pgp-general-encrypt, pgp-append-pubkey,
;;;|  pgp-sign-encrypt, pgp-decrypt, pgp-add-key      |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-wait-process-exit (proc)
  "Wait for the termination of a pgg asychronous process.
This is an active wait with 0.5 seconds pauses. Return an exit status."

;;; --------------- End function identification ----

  (let ((test t) ; global test condition for the main while
	(status)) ; status of the rocess
    (while test
      (setq status (process-status proc))
      (cond
       ((equal status 'run) ; running process, wait 0.5 second
	(if (input-pending-p)
	    (let ((even (read-event)))
	      (if (and (numberp even)
		       (= even 7))
					; at every step, check if a C-g
					; has been entered, if so, abort
		  (progn
		    (interrupt-process (process-name proc))
		    (if pgp-selected-seckey
					; clear the pass pharse if any
			(setq pgp-default-seckey (car pgp-selected-seckey)))
		    (setq pgp-selected-seckey nil)
		    (setq inhibit-quit nil))
		)))
	(sit-for 0 500))
       (t (setq test nil))
       ))
    (setq inhibit-quit nil)
    (cond
     ((equal status 'nil) 0)
     ((equal status 'exit) (process-exit-status proc))
     (t (delete-process proc) -1))
    ))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-output-filter        |
;;;| Purpose               : Filters and print the output of PGP
;;;| Calling parameters    :                          |
;;;|  proc  : the PGP process                         |
;;;|  string: new string to output                    |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-ro-protect, pgp-set-var-seckey,
;;;|  pgp-get-passphrase                              |
;;;| Calling functions    : pgp-list-secring, pgp-general-sign
;;;|  pgp-sign-encrypt, pgp-decrypt                   |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-output-filter (proc string)
  "Filters and print the output of PGP.
Scan the output for various strings corresponding to pgp questions and
messages. Take actions according to thoses messages. Maintains a point
to the last scan section so the same message will not be dected at each
call of the filter."

;;; --------------- End function identification ----

  (let ((old-buffer (current-buffer))
	(abort-process nil)
	(begpoint)
	(key-id)
	(even nil))
    (unwind-protect
	(let (moving)
	  (set-buffer (process-buffer proc))
	  (setq moving (= (point) (process-mark proc)))
	  (save-excursion
	    (goto-char (process-mark proc))
	    (if buffer-read-only
		(progn
		  (setq buffer-read-only nil)
		  (insert string)
		  (setq buffer-read-only t))
	      (insert string))
	    (set-marker (process-mark proc) (point)))
	  (if moving (goto-char (process-mark proc))))

      (if (re-search-backward pgp-strconst-end-ro pgp-buffer-search-point t)
					; End of "view only" decrypt
	  (progn
	    (setq even t)
	    (goto-char (process-mark proc))
	    (sit-for 0 500)
	    (process-send-string (process-name proc) "y\n")
	    ))

      (if (re-search-backward pgp-strconst-more-ro pgp-buffer-search-point t)
					; More line for "view only" decrypt
	  (progn
	    (setq even t)
	    (goto-char (process-mark proc))
	    (sit-for 0 100)
	    (process-send-string (process-name proc) " ")
	    ))
      
      (if (re-search-backward pgp-strconst-begin-ro pgp-buffer-search-point t)
					; Prompt for "view only" decrypt
	  (progn
					; NEED to protect the buffer first!!!
	    (setq buffer-read-only t)
	    (pgp-ro-protect)
	    (setq even t)
	    (goto-char (process-mark proc))
	    (sit-for 0 500)
	    (process-send-string (process-name proc) "y\n")
	    ))
      
      (if (re-search-backward pgp-strconst-double-bad-pass
			   pgp-buffer-search-point t)
					; Prompt for "view only" decrypt
	  (progn
	    (setq even t)
	    (goto-char (process-mark proc))
					; erase the pass phrase
	    (setq pgp-selected-seckey (cons
				       (car pgp-selected-seckey)
				       nil))
	    ))
      
      (if (search-backward pgp-strconst-enter-pass pgp-buffer-search-point t)
					; Enter pass phrase prompt
	  (progn
					; Read the key which pass is needed
	    (setq even t)
	    (if (re-search-backward pgp-strconst-pass-key nil t)
					; case of signing a message
		(progn
		  (search-forward "\"")
		  (setq begpoint (point))
		  (end-of-line)
		  (setq key-id (buffer-substring begpoint (1- (point)))))
					; case of decrypting
					; the messages are not the same
	      (progn
		(re-search-backward pgp-strconst-key-id nil t)
		(search-forward ": ")
		(setq begpoint (point))
		(end-of-line)
		(setq key-id (buffer-substring begpoint (point)))))
					; Read the key from the buffer
	    (pgp-set-var-seckey key-id)
	    (if  (re-search-forward pgp-strconst-bad-pass nil t)
					; Bad pass detected, clear pass
		(progn
		  (setq pgp-selected-seckey (cons
					     (car pgp-selected-seckey)
					     nil))
		  (message "PGP: bad pass phrase given for <%s>."
			   (car pgp-selected-seckey))
		  (ding)
		  (sit-for 2)
		  ))
					; Set the new seckey if needed
	    (if (not (pgp-get-passphrase))
					; Get the associated passphrase
		(progn
		  (interrupt-process (process-name proc))
		  (setq abort-process t)))
					; in case null pass phrase kill
					; the process
	    (goto-char (process-mark proc))
	    (if (not abort-process)
		(progn
		  (sit-for 0 500)
					; finally send the pass phrase
		  (process-send-string
		   (process-name proc) (cdr pgp-selected-seckey))
		  (process-send-string (process-name proc) "\n")))
	    ))
      
      (if (re-search-backward pgp-strconst-add-keyring
			   pgp-buffer-search-point t)
					; add key to keyring, needed only
					; to make the list of secring
	  (progn
	    (setq even t)
	    (goto-char (process-mark proc))
	    (sit-for 0 500)
	    (process-send-string (process-name proc) "n\n")
	    ))
      
      (if even
					; change the mark of the end
					; of the already examined portion
					; only if something interesting have
					; been found
	  (setq pgp-buffer-search-point (point)))
      (set-buffer old-buffer))))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-nil                  |
;;;| Purpose               : Do nothing, for null key binding only
;;;| Calling parameters    :                          |
;;;| Return value          : no significant           |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-ro-protect            |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-nil ()
  "Do nothing, for null key binding only."
  (interactive)

;;; --------------- End function identification ----

  (ding)
  (message "Command not allowed.")
  (sit-for 2))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-ro-protect           |
;;;| Purpose               : et protection on the current buffer when a
;;;|                         message is 'view-only'   |
;;;| Calling parameters    :                          |
;;;| Return value          : no significant           |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-nil                   |
;;;| Calling functions    : pgp-output-filter, pgp-show-status
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-ro-protect ()
  "Set protection on the current buffer when a message is 'view-only'.
This is the best level of protection I have been avble to think of till
now."

;;; --------------- End function identification ----

  (local-set-key "\C-k" 'pgp-nil)
  (local-set-key "\C-w" 'pgp-nil)
  (local-set-key [mouse-3] 'pgp-nil)
  (local-set-key [drag-mouse-1] 'pgp-nil)
  (local-set-key [down-mouse-1] 'pgp-nil)
  (local-set-key [M-mouse-2] 'pgp-nil)
  (local-set-key [M-mouse-3] 'pgp-nil)
  (local-set-key [M-down-mouse-1] 'pgp-nil)
  (local-set-key [M-drag-mouse-1] 'pgp-nil)
  (local-set-key [M-mouse-1] 'pgp-nil)
  (local-set-key [deleteline] 'pgp-nil)
  (local-set-key "\C-x\C-q" 'pgp-nil)
  (local-set-key "\C-x\C-s" 'pgp-nil)
  (local-set-key "\C-x\C-v" 'pgp-nil)
  (local-set-key "\C-x\C-w" 'pgp-nil)
  (local-set-key "\C-x\C-]" 'pgp-nil)
  (local-set-key "\C-x\C-(" 'pgp-nil)
  (local-set-key "\C-x\C-)" 'pgp-nil)
  (local-set-key "\C-xe" 'pgp-nil)
  (local-set-key "\C-xi" 'pgp-nil)
  (local-set-key "\C-xq" 'pgp-nil)
  (local-set-key "\C-xr" 'pgp-nil)
  (local-set-key "\C-xs" 'pgp-nil)
  (local-set-key "\C-xx" 'pgp-nil)
  (local-set-key "\C-x" 'pgp-nil)
  (local-set-key "\M-\C-k" 'pgp-nil)
  (local-set-key "\M-\C-w" 'pgp-nil)
  (local-set-key "\M-\C-" 'pgp-nil)
  (local-set-key "\M-'" 'pgp-nil)
  (local-set-key "\M-d" 'pgp-nil)
  (local-set-key "\M-k" 'pgp-nil)
  (local-set-key "\M-w" 'pgp-nil)
  (local-set-key "\M-x" 'pgp-nil)
  (local-set-key "\M-y" 'pgp-nil)
  (local-set-key "\M-|" 'pgp-nil)
  (local-set-key "\M-" 'pgp-nil)
  (local-set-key [menu-bar file save-buffer] 'pgp-nil)
  (local-set-key [menu-bar file write-file] 'pgp-nil)
  (local-set-key [menu-bar file print-buffer] 'pgp-nil)
  (local-set-key [menu-bar edit] 'pgp-nil)
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-list-pubring         |
;;;| Purpose               : Extract a list of public keys from pgp-pubring
;;;| Calling parameters    :                          |
;;;| Return value          : t or nil                 |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-output-buffer, |
;;;|  pgp-wait-process-exit                           |
;;;| Calling functions    :pgp-general-encrypt, pgp-sign-encrypt
;;;|  pgp-decr-mess, pgp-add-key                      |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-list-pubring ()
  "Extract a list of public keys from pgp-pubring.
This function is called only once during a Emacs session, the list is
updated automatically if new public keys are added to the pgp-pubring."

;;; --------------- End function identification ----

  (if (not pgp-list-pubring-set)
      (let ((process-connection-type t)
					; use PTYs for communication
	    (proc)
	    (old-buf (current-buffer))
	    (out-buf (pgp-create-output-buffer)))
	(set-buffer out-buf)
	(setq inhibit-quit t)
	(setq proc (start-process "pgp-process"
				  out-buf pgp-program "-kv"))
	(message "PGP: reading public keyring...")
	(process-kill-without-query proc)
	(set-marker (process-mark proc) (point))
	(process-send-string (process-name proc) "\n")
	(setq pgp-exit-status
	      (pgp-wait-process-exit proc))
	(set-buffer out-buf)
					; Wait for process termination
	(cond ((= pgp-exit-status 0)
					; pgp process exited noerror
	       (goto-char (point-min))
	       (search-forward "Type bits/keyID     Date       User ID\n")
	       (kill-region (point-min) (point))
					; Delete the begining of the result
	       (search-forward "key(s) examined.")
	       (beginning-of-line)
	       (kill-region (point) (point-max))
					; Delete the end of the result
	       (goto-char (point-min))
					; remove the disabled keys
	       (perform-replace "^pub@.*\n" "" nil t nil)
	       (goto-char (point-min))
	       (perform-replace "^pub .........................  "
				"" nil t nil)
	       (goto-char (point-min))
					; Delete begening of lines of key
	       (let ((pub-list nil)
		     (beg-line)
		     (end-line))
		 (while (progn
			  (beginning-of-line)
			  (setq beg-line (point))
			  (end-of-line)
			  (setq end-line (point))
					; get the bounds of current line
			  (setq pub-list (cons
					  (cons
					   (buffer-substring beg-line
							     end-line)
					   1)
					  pub-list))
					; add the current line to pub-list
			  (forward-line 1)
					; goto next line
			  (not (looking-at "^$"))))
		 (if (not pgp-list-keys)
		     (setq pgp-list-keys (cons (reverse pub-list) nil))
		   (setq pgp-list-keys (cons (reverse pub-list)
					     (cdr pgp-list-keys))))
					; Save the list in pgp-list-key
		 )
	       (erase-buffer)
	       (setq pgp-list-pubring-set t))
					; never do pgp-list-pubring again
	      (t
	       (message "PGP: reading public keyring aborted.")
	       (ding)
	       nil))
					; pgp process aborted
	(set-buffer old-buf)
	)
    t))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-list-secring         |
;;;| Purpose               : Extract a list of secret keys from pgp-secring
;;;| Calling parameters    :                          |
;;;| Return value          : t or nil                 |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-output-buffer, pgp-wait-process-exit
;;;| Calling functions    : pgp-get-seckey, set-set-seckey,
;;;|  pgp-set-var-seckey, pgp-decr-mess               |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;This one is a bit tricky, cf. PGP doc, run a pgp on the secring.pgp file
(defun pgp-list-secring ()
  "Extract a list of secret keys from pgp-secring.
This function is called only once during a Emacs session."

;;; --------------- End function identification ----

  (if (not pgp-list-secring-set)
      (let ((process-connection-type t)
					; use PTYs for communication
	    (proc)
	    (out-buf (pgp-create-output-buffer))
	    (pgp-path (getenv "PGPPATH")))
	(save-excursion
	  (set-buffer out-buf)
	  (setq inhibit-quit t)
	  (setq proc (start-process "pgp-process"
				    out-buf pgp-program
				    (concat pgp-path "/secring.pgp")))
	  (message "PGP: reading secret keyring...")
	  (process-kill-without-query proc)
	  (set-marker (process-mark proc) (point))
	  (setq pgp-buffer-search-point 1)
	  (set-process-filter proc 'pgp-output-filter)
	  (process-send-string (process-name proc) "\n")
	  (setq pgp-exit-status
		(pgp-wait-process-exit proc))
	  (set-buffer out-buf)
					; Wait for process termination
	  (cond ((= pgp-exit-status 0)
					; pgp process exited noerror
		 (goto-char (point-min))
		 (search-forward "Type bits/keyID     Date       User ID\n")
		 (kill-region (point-min) (point))
					; Delete the begining of the result
		 (search-forward "key(s) examined.")
		 (beginning-of-line)
		 (kill-region (point) (point-max))
					; Delete the end of the result
		 (goto-char (point-min))
		 (perform-replace "^sec ..........................."
				  "" nil t nil)
		 (goto-char (point-min))
					; Delete begening of lines of key
		 (let ((pub-list nil)
		       (beg-line)
		       (end-line))
		   (while (progn
			    (beginning-of-line)
			    (setq beg-line (point))
			    (end-of-line)
			    (setq end-line (point))
					; get the bounds of current line
			    (setq pub-list (cons
					    (cons
					     (buffer-substring beg-line
								end-line)
					     nil)
					    pub-list))
					; add the current line to pub-list
			    (forward-line 1)
					; goto next line
			    (not (looking-at "^$"))))
		   (if (not pgp-list-keys)
		       (setq pgp-list-keys (cons nil (reverse pub-list)))
					; need the reverse so the first is
					; pgp default
		     (setq pgp-list-keys (cons (car pgp-list-keys)
					       (reverse pub-list))))
					; Save the list in pgp-list-key
		   )
		 (erase-buffer)
		 (setq pgp-list-secring-set t))
					; never do pgp-list-secring again
		(t
		 (message "PGP: reading secret keyring aborted.")
		 (ding)
		 nil))
					; pgp process aborted
	  )
	)
    t))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-try-completion       |
;;;| Purpose               : Return the first element of the list matching str
;;;| Calling parameters    :                          |
;;;|  str : the element to match                      |
;;;|  list: the list to match str against             |
;;;| Return value          : matching string or nil   |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : RECURSIVE                 |
;;;| Calling functions    : pgp-find-first-match, pgp-collection
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-try-completion (str list)
  "Return the first element of the list matching str."

;;; --------------- End function identification ----

  (if list
      (if (string-match str (car (car list)))
	  (car (car list))
	(pgp-try-completion str (cdr list)))
    nil))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-all-completion       |
;;;| Purpose               : Return a list of elements matching str
;;;| Calling parameters    :                          |
;;;|  str : the element to match                      |
;;;|  list: the list to match str against             |
;;;| Return value          : the list or nil          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : RECURSIVE                 |
;;;| Calling functions    : pgp-collection            |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-all-completion (str list)
  "Return a list of elements matching str."

;;; --------------- End function identification ----

  (if list
      (if (string-match str (car (car list)))
	  (cons (car (car list)) (pgp-all-completion str (cdr list)))
	(pgp-all-completion str (cdr list)))
    nil))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-set-initial          |
;;;| Purpose               : Try to find a good entry in the list
;;;|                         matching name            |
;;;| Calling parameters    :                          |
;;;|  name: the name to find matching for             |
;;;| Return value          : a string matching        |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-read-recipient        |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;; This function is not implemented yet.

;;; Given the email address and the list of public keys, it should try
;;; to guess which pgp key correspond to the given email address. This
;;; should be quite a straight process when the pgp key contains the email
;;; address (but require some works anyway when the email is local but the
;;; address given in the key is fully defined) but what to do when the
;;; pgp public key is only the user name?

(defun pgp-set-initial (name)
  "Try to find a good entry in the list matching name."

;;; --------------- End function identification ----

  (if (string= name "")
      ""
    ""))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-read-recipient       |
;;;| Purpose               : Read from minibuffer a PGP key for a given prompt
;;;| Calling parameters    :                          |
;;;|  prompt: a prompt string to display              |
;;;|  list-coll: a list of all possible choices       |
;;;|  name: to be inserted in the prompt              |
;;;| Return value          : a completed key          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-find-first-match, pgp-find-first-exit
;;;|  pgp-restore-map, pgp-mouse-choose, pgp-set-initial,
;;;|  pgp-collection                                  |
;;;| Calling functions    : pgp-set-seckey, pgp-list-recipient-id,
;;;|  pgp-general-encrypt, pgp-sign-encrypt           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-read-recipient (prompt list-coll name)
  "Read from minibuffer a PGP key for a given prompt.
This function uses the PGP keys lists and implements completion:
SPC   find the first matching ID
TAB   find the first matching ID and exit
RET   find the first matching ID and exit
LFD   find the first matching ID and exit
C-g   abort
C-n   find next matching
?     give a list of possible matching ID"

;;; --------------- End function identification ----

  (let ((copmap (copy-keymap minibuffer-local-completion-map))
	(ret))
    (define-key minibuffer-local-completion-map " " 'pgp-find-first-match)
    (define-key minibuffer-local-completion-map "\C-m" 'pgp-find-first-exit)
    (define-key minibuffer-local-completion-map "\C-i" 'pgp-find-first-exit)
    (define-key minibuffer-local-completion-map "\C-j" 'pgp-find-first-exit)
    (define-key minibuffer-local-completion-map "\C-g" 'pgp-restore-map)
    (define-key completion-list-mode-map [mouse-2] 'pgp-mouse-choose)
    (setq pgp-list-completion list-coll)
    (setq ret (completing-read prompt 'pgp-collection nil nil
			       (pgp-set-initial name)))
    (setq minibuffer-local-completion-map (copy-keymap copmap))
    (define-key completion-list-mode-map [mouse-2] 'mouse-choose-completion)
    ret
    )
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-restore-map          |
;;;| Purpose               : Restore minibuffer-local-completion-map
;;;|                         before exiting           |
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-read-recipient        |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-restore-map ()
  "Restore minibuffer-local-completion-map before exiting."
  (interactive)

;;; --------------- End function identification ----

  (define-key minibuffer-local-completion-map " " 'minibuffer-complete-word)
  (define-key minibuffer-local-completion-map "\C-m" 'exit-minibuffer)
  (define-key minibuffer-local-completion-map "\C-i" 'minibuffer-complete)
  (define-key minibuffer-local-completion-map "\C-j" 'exit-minibuffer)
  (define-key minibuffer-local-completion-map "\C-g" 'abort-recursive-edit)
  (define-key completion-list-mode-map [mouse-2] 'mouse-choose-completion)
  (abort-recursive-edit))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-find-first-match     |
;;;| Purpose               : Find the first PGP recipient ID that match
;;;|                         the string               |
;;;| Calling parameters    :                          |
;;;| Return value          : a completed key or nil   |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-try-completion        |
;;;| Calling functions    : pgp-read-recipient, pgp-find-first-exit
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-find-first-match ()
  "Find the first PGP recipient ID that match the string."
  (interactive)

;;; --------------- End function identification ----

  (let ((str-compl (if (> (point-max) (point-min))
					; try a completion with the contents
					; of the minibuffer
		       (pgp-try-completion
			(buffer-substring (point-min) (point-max))
			pgp-list-completion)
					; else propose completion
		     (progn
		       (minibuffer-completion-help)
		       ""))))
    (if str-compl
					; input is valid, re-display
	(progn
	  (goto-char (point-min))
	  (if (> (point-max) (point-min))
	      (delete-region (point-min) (point-max)))
	  (insert str-compl)
	  t)
					; input is not valid
      (progn
	(ding)
	(save-excursion
	  (goto-char (point-max))
	  (save-excursion
	    (insert " [No completions]"))
	  (sit-for 2)
	  (kill-line))
	nil))
    )
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-find-first-exit      |
;;;| Purpose               : Find the first PGP recipient ID that match
;;;|                         the string and exit      |
;;;| Calling parameters    :                          |
;;;| Return value          : a completed key or nil   |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-find-first-match      |
;;;| Calling functions    : pgp-read-recipient        |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-find-first-exit ()
  "Find the first PGP recipient ID that match the string and exit."
  (interactive)

;;; --------------- End function identification ----

  (if (pgp-find-first-match)
      (exit-minibuffer))
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-collection           |
;;;| Purpose               : Create the alist collection of matching PGP keys
;;;| Calling parameters    :                          |
;;;|  str : the string to complete                    |
;;;|  func: not used but needed                       |
;;;|  flag: set return a list, nil a single element   |
;;;| Return value          : a list or the first      |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-try-completion, pgp-all-completion
;;;| Calling functions    : pgp-read-recipient        |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;; It is a programmed completion function as decribed in GNE Emacs Lisp
;;; Reference Manual (17.5.2)
(defun pgp-collection (str func flag)
  "Create the alist collection of matching PGP keys."

;;; --------------- End function identification ----

  (if flag 
      (pgp-all-completion str pgp-list-completion)
    (pgp-try-completion str pgp-list-completion)
    )
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-list-recipient-id    |
;;;| Purpose               : Make a string of quoted PGP recipient ID
;;;| Calling parameters    :                          |
;;;|  mail-list: list of name to find pgp-key for     |
;;;| Return value          : list of key or nil       |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : RECURSIVE, pgp-read-recipient
;;;| Calling functions    : pgp-general-encrypt, pgp-sign-encrypt
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-list-recipient-id (mail-list)
  "Make a string of quoted PGP recipient ID."

;;; --------------- End function identification ----

  (if mail-list
      (let ((key (pgp-read-recipient (concat "Enter PGP recipient ID for <"
					     (car mail-list) ">: ")
				     (car pgp-list-keys)
				     (car mail-list))))
	(if (not (string= key ""))
					; never keep empty keys
	    (cons  key (pgp-list-recipient-id (cdr mail-list)))
	  (pgp-list-recipient-id (cdr mail-list))))
    nil))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-mouse-choose         |
;;;| Purpose               : Click on an alternative in the `*Completions*'
;;;|                         buffer to choose it      |
;;;| Calling parameters    :                          |
;;;|  event: a mouse event                            |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-read-recipient        |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;Stolen from mouse.el
(defun pgp-mouse-choose (event)
  "Click on an alternative in the `*Completions*' buffer to choose it."
  (interactive "e")

;;; --------------- End function identification ----

  (let (choice)
    (save-excursion
      (set-buffer (window-buffer (posn-window (event-start event))))
      (save-excursion
	(goto-char (posn-point (event-start event)))
	(skip-chars-backward "^ \t\n")
	(let ((beg (point)))
	  (skip-chars-forward "^ \t\n")
	  (setq choice (buffer-substring beg (point))))))
    (set-buffer (window-buffer (minibuffer-window)))
    (goto-char (point-min))
    (if (> (point-max) (point-min))
	(delete-region (point-min) (point-max)))
    (insert choice)
    (minibuffer-complete-and-exit)))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-clear-sign           |
;;;| Purpose               : Clear-sign a mail message|
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-general-sign          |
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-clear-sign ()
  "Clear-sign a mail message."
  (interactive)

;;; --------------- End function identification ----

  (pgp-general-sign t))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-sign                 |
;;;| Purpose               : Sign a mail message      |
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-general-sign          |
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-sign ()
  "Sign a mail message."
  (interactive)

;;; --------------- End function identification ----

  (pgp-general-sign nil))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-general-sign         |
;;;| Purpose               : General function to sign a mail message
;;;| Calling parameters    :                          |
;;;|  clear: flag to clear-sign or not                |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-output-buffer, pgp-get-passphrase,
;;;|  pgp-wait-process-exit, pgp-output-filter, pgp-show-status,
;;;| pgp-update-pass                                  |
;;;| Calling functions    : pgp-sign, pgp-clear-sign  |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-general-sign (clear)
  "General function to sign a mail message.
Clear is a boolean tha means if set to non-nil that a clear-signature should
be generated."

;;; --------------- End function identification ----

  (let ((old-buffer (current-buffer))
	(out-buf (pgp-create-output-buffer))
	(beg)
	(end)
	(proc))
    (save-excursion
      (set-buffer out-buf)
      (if (pgp-get-passphrase)
	  (progn
	    (set-buffer out-buf)
					; bind C-g to kill pgp process
	    (setq inhibit-quit t)
	    (setq proc
		  (if clear
		      (start-process "pgp-process"
				     out-buf pgp-program
				     "-saftu" (car pgp-selected-seckey)
				     "+clearsig=on")
		    (start-process "pgp-process"
				   out-buf pgp-program
				   "-saftu" (car pgp-selected-seckey))))
	    (message "PGP: signing message...")
	    (process-kill-without-query proc)
	    (set-marker (process-mark proc) (point))
	    (setq pgp-buffer-search-point (point-min))
	    (set-process-filter proc 'pgp-output-filter)
	    (set-buffer old-buffer)
					; goto mail buffer
	    (goto-char (point-min))
	    (search-forward mail-header-separator)
	    (forward-char)
	    (setq beg (point))
					; extract the message
	    (sit-for 0 500)
	    (process-send-region (process-name proc) beg (point-max))
	    (if (not (string= (buffer-substring (1- (point-max)) (point-max))
			      "\n"))
		(process-send-string (process-name proc) "\n")
					; Add a \n at the end of buffer
		)
	    (process-send-string (process-name proc) "")
					; send the message to pgp, add ^D
	    
	    (set-buffer out-buf)
	    (setq pgp-exit-status
		  (pgp-wait-process-exit proc))
					; Wait for process termination
	    (cond ((= pgp-exit-status 0)
					; pgp process exited noerror
		   (set-buffer old-buffer)
		   (delete-region beg (point-max))
					; remove the original message
		   (set-buffer out-buf)
		   (if clear
		       (search-backward "-----BEGIN PGP SIGNED MESSAGE-----")
		     (search-backward "-----BEGIN PGP MESSAGE-----"))
		   (setq beg (point))
		   (if clear
		       (search-forward "-----END PGP SIGNATURE-----")
		     (search-forward "-----END PGP MESSAGE-----"))
		   (forward-char 1)
		   (setq end (point))
		   (set-buffer old-buffer)
		   (insert-buffer-substring out-buf beg end)
					; copy the pgp result
		   (pgp-show-status "s")
		   )
		  ((= pgp-exit-status 20)
		   (message "PGP: wrong pass phrase given too many times.")
		   (ding)
		   (setq pgp-selected-seckey (cons
					      (car pgp-selected-seckey)
					      nil))
					; erase the wrong pass, it will erase
					; in the global list too
		   )
		  (t
		   (message "PGP: signature aborted.")
		   (ding))
		  )
	    (pgp-update-pass))
	)
      (set-buffer old-buffer)
					; just in case, restore mail buffer
      )
    ))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-encrypt              |
;;;| Purpose               : Enrypt a mail message    |
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-append-pubkey, pgp-general-encrypt,
;;;|  pgp-create-temp-buffer                          |
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-encrypt ()
  "Encrypt a mail message."
  (interactive)

;;; --------------- End function identification ----

  (pgp-create-temp-buffer)
  (if pgp-auto-add-pubkey
					; try to automatically add pubkey
      (save-excursion
	(if (pgp-append-pubkey t)
	    (progn
	      (goto-char (point-min))
	      (search-forward mail-header-separator)
	      (forward-char 1)
	      (pgp-general-encrypt nil (point) (point-max)))))
    (save-excursion
      (goto-char (point-min))
      (search-forward mail-header-separator)
      (forward-char 1)
      (pgp-general-encrypt nil (point) (point-max)))))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-encrypt-view         |
;;;| Purpose               : Encrypt a mail message for view-only
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-temp-buffer, pgp-general-encrypt
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-encrypt-view ()
  "Encrypt a mail message for view-only."
  (interactive)

;;; --------------- End function identification ----

  (pgp-create-temp-buffer)
  (save-excursion
    (goto-char (point-min))
    (search-forward mail-header-separator)
    (forward-char 1)
    (pgp-general-encrypt t (point) (point-max))))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-encrypt-region       |
;;;| Purpose               : Encrypt a mail message for viewthe region
;;;|                  defined between mark and point  |
;;;| Calling parameters    :                          |
;;;|  beg: the beginning of the region                |
;;;|  end: the end of the region                      |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-temp-buffer, pgp-general-encrypt
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-encrypt-region (beg end)
  "Encrypt a mail message for viewthe region defined between mark and point."
  (interactive "r")

;;; --------------- End function identification ----

  (pgp-create-temp-buffer)
  (save-excursion
    (pgp-general-encrypt nil beg end)))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-general-encrypt      |
;;;| Purpose               : General function to encrypt a mail message
;;;| Calling parameters    :                          |
;;;|  view: if set, encrypt for view only             |
;;;|  begreg, endreg: limit of the region to encrypt  |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-list-pubring, pgp-list-recipient-id,
;;;|  pgp-extract-email-recipient, pgp-set-key, pgp-read-recipient,
;;;|  pgp-create-output-buffer, pgp-wait-process-exit, pgp-show-status
;;;| Calling functions    : pgp-encrypt, pgp-encrypt-region,
;;;| pgp-encrypt-view                                 |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-general-encrypt (view begreg endreg)
  "General function to encrypt a mail message.
View is a boolean that if set to non-nil means the message should
be crypted with view-only option. The message is crypted between the 
begreg and endreg."

;;; --------------- End function identification ----

  (let ((old-buffer (current-buffer))
	(out-buf)
	(beg)
	(end)
	(proc)
	(recp-list nil)
	(key-read ""))
    (if (pgp-list-pubring)
	(progn
	  (setq recp-list (pgp-list-recipient-id
			   (pgp-extract-email-recipient)))
					; list of encrypt keys extracted
					; from the list of the mail recipients
	  (if (and pgp-self-crypt
		   (pgp-get-seckey))
					; encrypt with user's own key
	      (setq recp-list (append recp-list
				      (list (car pgp-selected-seckey))))
	    )
	  (while (not (string=
		       (setq key-read
			     (pgp-read-recipient
			      "Encrypt the message for someone else: "
			      (car pgp-list-keys) ""))
		       ""))
	    (setq recp-list (append recp-list (list key-read)))
	    )
	  (if recp-list
	      (progn
		(setq out-buf (pgp-create-output-buffer))
		(set-buffer out-buf)
		(setq inhibit-quit t)
		(if view
					; view-only encrypt
		    (setq proc (eval (append
				      (list 'start-process "pgp-process"
					    out-buf pgp-program
					    "-eamf")
				      recp-list)))
					; standrad encrypt
		  (setq proc (eval (append
				    (list 'start-process "pgp-process"
					  out-buf pgp-program
					  "-eaf")
				    recp-list)))
		  )
		(message "PGP: encrypting message...")
		(process-kill-without-query proc)
		(set-marker (process-mark proc) (point))
		(set-buffer old-buffer)
					; goto mail buffer
					; extract the message
		(sit-for 0 500)
		(process-send-region (process-name proc) begreg endreg)
		(if (not (string=
			  (buffer-substring (1- endreg) endreg)
			  "\n"))
		    (process-send-string (process-name proc) "\n")
					; Add a \n at the end of buffer
		  )
					; This is used to automatically insert
					; pubkey
		(set-buffer "*PGP temp*")
		(if (> (point-max) (point-min))
					; There is a saved pubkey to add
		    (progn
		      (process-send-string (process-name proc) "\n")
		      (process-send-region (process-name proc) (point-min)
					   (point-max)))
		    )
		(erase-buffer)
		(set-buffer old-buffer)
		(process-send-string (process-name proc) "")
					; send the message to pgp, add ^D
		
		(set-buffer out-buf)
		(setq pgp-exit-status
		      (pgp-wait-process-exit proc))
					; Wait for process termination
		(cond ((= pgp-exit-status 0)
					; pgp process exited noerror
		       (set-buffer old-buffer)
		       (delete-region begreg endreg)
					; remove the original message
		       (set-buffer out-buf)
		       (search-backward "-----BEGIN PGP MESSAGE-----")
		       (setq beg (point))
		       (search-forward "-----END PGP MESSAGE-----")
		       (forward-char 1)
		       (setq end (point))
		       (set-buffer old-buffer)
		       (goto-char begreg)
		       (insert-buffer-substring out-buf beg end)
					; copy the pgp result
		       (pgp-show-status "e")
		       )
		      (t
		       (message "PGP: encrypt aborted.")
		       (ding))
		      )
		)
	    (progn
	      (message "PGP: no encrypt key defined.")
	      (ding))
	    )))
    (set-buffer old-buffer)
					; just in case, restore mail buffer
    ))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-append-pubkey        |
;;;| Purpose               : Append the public key to the mail message
;;;| Calling parameters    :                          |
;;;|  &status: display the status or not              |
;;;| Return value          : t or nil                 |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-output-buffer, pgp-get-seckey,
;;;|  pgp-wait-process-exit, pgp-show-status, pgp-create-temp-buffer
;;;| Calling functions    : pgp-encrypt               |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-append-pubkey (&optional status)
  "Append the public key to the mail message.
The public key is the one corresponding to the selected secret key. It is
appended at the current point. If status is set non-nil show the status
after the run."
  (interactive)

;;; --------------- End function identification ----

  (let ((old-buffer (current-buffer))
	(out-buf (pgp-create-output-buffer))
	(beg)
	(end)
	(begreg (point))
	(proc)
	(recp-list nil)
	(key-read ""))
    (save-excursion
      (if (pgp-get-seckey)

	  (progn
	    (set-buffer out-buf)
	    (setq inhibit-quit t)
	    (setq proc (start-process "pgp-process"
				    out-buf pgp-program
				    "-kxfa"
				    (car pgp-selected-seckey)))
	    (message "PGP: extracting public key...")
	    (process-kill-without-query proc)
	    (set-marker (process-mark proc) (point))
	    (setq pgp-exit-status
		  (pgp-wait-process-exit proc))
					; Wait for process termination
	    (set-buffer out-buf)
	    (cond ((= pgp-exit-status 0)
					; pgp process exited noerror
		   (search-backward "-----BEGIN PGP PUBLIC KEY BLOCK-----")
		   (setq beg (point))
		   (search-forward "-----END PGP PUBLIC KEY BLOCK-----")
		   (forward-char 1)
		   (setq end (point))
		   (set-buffer old-buffer)
		   (goto-char begreg)
		   (if (not status)
					; interactive call
		       (progn 
			 (insert-buffer-substring out-buf beg end)
					; copy the pgp result
			 (pgp-show-status "s"))
		     (progn
		       (set-buffer (pgp-create-temp-buffer))
		       (insert-buffer-substring out-buf beg end)
		       (set-buffer old-buffer)))
		   )
		  (t
		   (message "PGP: extract public key aborted.")
		   (ding))
		  )))
      (set-buffer old-buffer)
					; just in case, restore mail buffer
      )))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-sign-encrypt         |
;;;| Purpose               : Sign and crypt a mail message
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-output-buffer, pgp-get-passphrase,
;;;|  pgp-list-pubring, pgp-list-recipient-id, pgp-extract-email-recipient,
;;;|  pgp-get-seckey, pgp-read-recipient, pgp-wait-process-exit,
;;;|  pgp-show-status, pgp-update-pass                |
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-sign-encrypt ()
  "Sign and crypt a mail message."
  (interactive)

;;; --------------- End function identification ----

  (let ((old-buffer (current-buffer))
	(out-buf (pgp-create-output-buffer))
	(beg)
	(recp-list nil)
	(key-read)
	(end)
	(proc))
    (save-excursion
      (if (and (pgp-get-passphrase)
	       (pgp-list-pubring))
	  (progn
	    (setq recp-list (pgp-list-recipient-id
			     (pgp-extract-email-recipient)))
					; list of encrypt keys extracted
					; from the list of the mail recipients
	    (if (and pgp-self-crypt
		     (pgp-get-seckey))
					; encrypt with user's own key
		(setq recp-list (append recp-list
					(list (car pgp-selected-seckey))))
	      )
	    (while (not (string=
			 (setq key-read
			       (pgp-read-recipient
				"Encrypt the message for someone else: "
				(car pgp-list-keys) ""))
			 ""))
	      (setq recp-list (append recp-list (list key-read)))
	      )
	    (if recp-list
		(progn
		  (set-buffer out-buf)
		  (setq inhibit-quit t)
		  (setq proc (eval (append
				    (list 'start-process "pgp-process"
					  out-buf pgp-program
					  "-seafu" (car pgp-selected-seckey))
				    recp-list)))
		  (message "PGP: encrypting message...")
		  (process-kill-without-query proc)
		
		  (setq pgp-buffer-search-point (point-min))
		  (set-process-filter proc 'pgp-output-filter)
		  (set-buffer old-buffer)
					; goto mail buffer
		  (goto-char (point-min))
		  (search-forward mail-header-separator)
		  (forward-char)
		  (setq beg (point))
					; extract the message
		  (sit-for 0 500)
		  (process-send-region (process-name proc) beg (point-max))
		  (if (not (string=
			    (buffer-substring (1- (point-max)) (point-max))
			    "\n"))
		      (process-send-string (process-name proc) "\n")
					; Add a \n at the end of buffer
		    )
		  (process-send-string (process-name proc) "")
					; send the message to pgp, add ^D
		  
		  (set-buffer out-buf)
		  (setq pgp-exit-status
			(pgp-wait-process-exit proc))
					; Wait for process termination
		  (cond ((= pgp-exit-status 0)
					; pgp process exited noerror
			 (set-buffer old-buffer)
			 (delete-region beg (point-max))
					; remove the original message
			 (set-buffer out-buf)
			 (search-backward "-----BEGIN PGP MESSAGE-----")
			 (setq beg (point))
			 (search-forward "-----END PGP MESSAGE-----")
			 (forward-char 1)
			 (setq end (point))
			 (set-buffer old-buffer)
			 (insert-buffer-substring out-buf beg end)
					; copy the pgp result
			 (pgp-show-status "s")
			 )
			((= pgp-exit-status 20)
			 (message
			  "PGP: wrong pass phrase given too many times.")
			 (ding)
			 (setq pgp-selected-seckey (cons
						    (car pgp-selected-seckey)
						    nil))
					; erase the wrong pass, it will erase
					; in the global list too
			 )
			(t
			 (message "PGP: encrypting aborted.")
			 (ding))
			)
		  (pgp-update-pass))
	      (progn
		(message "PGP: no encrypt key defined.")
		(ding))
	)))
      (set-buffer old-buffer)
					; just in case, restore mail buffer
      )
    ))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-show-status          |
;;;| Purpose               : Display the *PGP output* buffer
;;;| Calling parameters    :                          |
;;;|  str: specify the kind of output                 |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-create-temp-buff, pgp-ro-protect,
;;;|  pgp-print-return-message                        |
;;;| Calling functions    : pgp-general-sign, pgp-general-encrypt,
;;;|  pgp-append-pubkey, pgp-sign-encrypt, pgp-decr-mess,
;;;|  pgp-add-key                                     |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-show-status (str)
  "Display the *PGP output* buffer.
Clean the buffer then display it. In case of view-only message, another
function is used. Str is used to switch the cleaning according to the 
performed operation.
Side effect in case of add key to the pubring: update the internal list."
  
;;; --------------- End function identification ----
  
  (let ((buf (current-buffer))
	(curp)
	(where)
	(end-buf)
	(test)
	(end-search)
	(temp-buf)
	(out-buf)
	(new-key 0)
	(new-sig 0)
	(string))
    (set-buffer "*PGP process*")
					; Clean the *PGP process* buffer
    (goto-char (point-min))
    (forward-line 3)
    (setq curp (point))
					; Keep the PGP header
    
;;; General case of Encrypt, sign or extract key
    (if (or (string= str "e") (string= str "s")
	    (string= str "x"))
					; This is done in case of signature
					; or crypting
	(progn
	  (forward-line 1)
	  (setq curp (point))
	  (while (re-search-forward pgp-strconst-key-id nil t)
					; search for key lines
	    (beginning-of-line)
	    (if (> (point) curp)
		(delete-region curp (point)))
	    (cond ((string= str "s")
		   (insert "* Signed with following key:\n")
		   (setq str "e"))
		  ((string= str "e")
		   (insert "* Crypted for following key(s):\n")
		   (setq str ""))
		  ((string= str "x")
		   (insert "* Following key extracted:\n")
		   (setq str ""))
		  )
	    (forward-line 2)
					; Keep the 2 lines with the key
	    (insert "\n")
	    (setq curp (point))
	    )
	  (delete-region curp (point-max)))
      )
;;; General decrypt
    (if (string= str "d")
					; standard decrypt
	(progn
					; FIRST, move the view-only parts
	  
	  (setq out-buf (current-buffer))
	  (setq temp-buf (pgp-create-temp-buffer))
	  (set-buffer temp-buf)
	  (pgp-ro-protect)
	  (set-buffer out-buf)
	  (setq inhibit-quit t)
	  (while (re-search-forward pgp-strconst-begin-ro nil t)
					; first protect
	    (setq curp (point))
	    (setq end-buf (re-search-forward pgp-strconst-end-ro))
	    (set-buffer temp-buf)
	    (setq buffer-read-only nil)
	    (insert-buffer-substring out-buf curp end-buf)
					; copy to *PGP temp*
	    (setq buffer-read-only t)
	    (set-buffer out-buf)
	    (goto-char curp)
	    (setq buffer-read-only nil)
	    (delete-region curp end-buf)
					; remove from *PGP process*
	    (forward-line -1)
	    (setq curp (point))
	    (forward-line 3)
	    (delete-region curp (point))
	    (insert
	     "WARNING:\nThe message is marked \"For your eyes only\".\n")
	    (insert "It have been moved to the buffer *PGP temp*.\n\n")
	    (setq buffer-read-only t)
	    )
	  (setq inhibit-quit nil)
	  (setq buffer-read-only nil)
	  (goto-char (point-min))
	  (forward-line 3)
	  (insert "\n")
	  (setq curp (point))
	  (setq test t)
					; find the nature of the next packet
	  (while test
	    (setq test nil)
	    (setq where (point-max))
	    (if (re-search-forward pgp-strconst-crypted-mess nil t)
		(progn
		  (setq where (point))
		  (setq test "e")))
	    (goto-char curp)
	    (if (re-search-forward pgp-strconst-signed-mess nil t)
		(progn
		  (setq where (min where (point)))
		  (if (= where (point))
		      (setq test "s"))))
	    (goto-char curp)
	    (if (re-search-forward pgp-strconst-key-mess nil t)
		(progn
		  (setq where (min where (point)))
	          (if (= where (point))
		      (setq test "a"))))
	    (if (re-search-forward pgp-strconst-bad-ascii nil t)
		(progn
		  (setq where (min where (point)))
		  (if (= where (point))
		      (setq test "ba"))))
	    (goto-char where)
					; is that the last packet?
	    (if
		(setq end-buf
		      (re-search-forward pgp-strconst-packet-sep nil t))
		(goto-char where))
					; do according to the nature of packet
	    (cond
					; for encrypt
	     ((string= test "e")
	      (forward-line 1)
	      (delete-region curp (point))
	      (if (re-search-forward pgp-strconst-good-pass end-buf t)
					; Good pass phrase
		  (progn
		    (goto-char curp)
		    (insert "* Message encrypted for following key:\n")
		    (forward-line 2)
		    (insert "\n")
		    (setq curp (point))
		    (re-search-forward pgp-strconst-good-pass)
		    (delete-region curp (point))
		    )
		(progn
		  (if (re-search-forward pgp-strconst-bad-pass end-buf t)
					; Bad pass phrase
		      (progn
			(goto-char curp)
			(insert "* Message encrypted for following key:\n")
			(forward-line 3)
			(insert
			 "Pass phrase have been given incorrectely too many times.\n\n")
			(setq curp (point))
			
			)
					; Encrypted for other
		    (progn
		      (insert "* Message encrypted for the following key:\n")
		      (setq curp (point))
		      (end-of-line)
		      (forward-char 2)
		      (delete-region curp (point))
		      (insert "Key for user ID:")
		      (forward-line 2)
		      (delete-char 1)
		      (forward-line 1)
		      (insert "\n")
		      (setq curp (point))
		      )
		    )
					; clear end of message
		  (if (re-search-forward pgp-strconst-packet-sep nil t)
		      (progn
			(forward-line -1)
			(delete-region curp (point))
			)
		    (progn
		      (goto-char (point-max))
		      (forward-line -2)
		      (delete-region curp (point))
		      ))
		  (setq curp (point))
		  )
		)
	      )
	     ((string= test "s")
					; for sign or sign-encrypt
	      (forward-line 1)
	      (delete-region curp (point))
	      (setq curp (point))
	      (if (re-search-forward pgp-strconst-good-sign end-buf t)
					; Signed with a valid key
		  (progn
		    (goto-char curp)
		    (insert "* Signed with following key:\n")
		    (forward-line 2)
		    (insert "\n")
		    (setq curp (point))
		    )
					; signed with an invalid key
		(if (re-search-forward pgp-strconst-bad-sign end-buf t)
		    (progn
		      (goto-char curp)
		      (re-search-forward pgp-strconst-bad-sign end-buf t)
		      (delete-region curp (point))
		      (insert "WARNING:\n")
		      (insert
		       "Message signed with an unkown key, signature cannot be checked.\n")
		      (setq curp (point))
		      )
					; message altered, sig OK
		  (progn
		    (forward-line 2)
		    (delete-region curp (point))
		    (insert "* Signed with following key:\n")
		    (forward-line 2)
		    (insert "\nWARNING:\n")
		    (insert
		     "The contents of the message seams to have been tempered with.\n\n")
		    (setq curp (point))
		    )))
	      )
					; altered message, unreadable
	     ((string= test "ba")
					;
	      (forward-line -2)
	      (insert
	       "The message seams to have been altered during transportation.\n\n")
	      (insert "The remainder of the message cannot be decrypted.\n")
	      (setq curp (point))
	      (goto-char (point-max))
	      (forward-line -2)
	      (delete-region curp (point)))
					; key message
	     ((string= test "a")
					; this is a key message
	      (forward-line 1)
	      (delete-region curp (point))
	      (insert "* Message containing the following public key(s)\n")
	      (setq curp (point))
	      (setq end-search (re-search-forward pgp-strconst-end-key-mess))
	      (goto-char curp)
	      (while (re-search-forward
		      "^pub +\\([0-9]+\\)/\\([0-9ABCDEF]+\\) \\(199[0-9]/[0-1][0-9]/[0-3][0-9]\\) \\(.*\\)$"
		      end-search t)
		(beginning-of-line)
		(replace-match
		 "Key for user ID: \\4\n\\1-bit key, Key ID \\2, created \\3" t)
		(setq curp (point))
		(setq end-search
		      (re-search-forward pgp-strconst-end-key-mess))
		(goto-char curp))
	      (search-backward "* Message")
	      (setq curp (point))
	      (while (re-search-forward
		      "^sig +\\([0-9ABCDEF]+\\) .*Unknown signator,.*$"
		      end-search t)
		(beginning-of-line)
		(replace-match
		 "- signed with key ID \\1, for unknown signator" t)
		(setq curp (point))
		(setq end-search
		      (re-search-forward pgp-strconst-end-key-mess))
		(goto-char curp))
	      (forward-line 1)
	      (search-backward "* Message")
	      (setq curp (point))
	      (while (re-search-forward
		      "^sig +\\([0-9ABCDEF]+\\) +\\(.*\\)$" end-search t)
		(beginning-of-line)
		(replace-match
		 "- signed with key ID \\1, for user ID: \\2" t)
		(setq curp (point))
		(setq end-search
		      (re-search-forward pgp-strconst-end-key-mess))
		(goto-char curp))
	      (goto-char end-search)
	      (beginning-of-line)
	      (delete-region (point) end-search)
	      (setq curp (point))
	      (insert
	       "\nTo add the key(s) to your public keyring, type \"C-c a\" in RMAIL buffer.\n\n")
	      )
	     )
					; add packet separator
	    (if (re-search-forward pgp-strconst-packet-sep nil t)
		(progn
		  (beginning-of-line)
		  (setq curp (point))
		  (end-of-line)
		  (delete-region curp (point))
		  (insert
		   "-------------------- Next packet --------------------\n\n")
		  (setq curp (point))
		  )
	      (progn
		(goto-char (point-max))
		(setq test nil)
		(forward-line -2)
		(delete-region (point) (point-max)))))
					; Clean *PGP temp*
	  (set-buffer temp-buf)
					; set the view-only buffer
	  (if (> (buffer-size) 0)
	      (progn
					; separate the messages
		(goto-char (point-min))
		(setq inhibit-quit t)
		(setq buffer-read-only nil)
		(insert
		 "WARNING:\nSome Emacs functions in this buffer have been disabled ")
		(insert "so the buffer cannot\nbe saved.")
		(insert
		 "Hey! This is a message marked \"For your eyes only\" after all!\n\n")
		(setq curp (point))
		(while
		    (progn
		      (forward-line 6)
		      (delete-region curp (point))
		      (re-search-forward pgp-strconst-end-ro)
		      (setq curp (point))
		      (beginning-of-line)
		      (delete-region (point) curp)
		      (setq curp (point))
		      (if (re-search-forward pgp-strconst-end-ro nil t)
			  (progn
			    (goto-char curp)
			    (insert
			     "-------------------- Next packet --------------------\n\n")
			    (setq curp (point))
			    t))))
		(goto-char (point-min))
					; remove the "More" lines
		(while (re-search-forward pgp-strconst-more-ro nil t)
		  (setq curp (point))
		  (beginning-of-line)
		  (delete-region (point) curp))
		(goto-char (point-min))
		(setq buffer-read-only t)
		(set-buffer-modified-p nil)
		(setq inhibit-quit nil)
		))
	  ))
;;; General add key to pubring
    (if (string= str "a")
					;add key to pubring
	(progn
	  (forward-line 1)
	  (if (re-search-forward pgp-strconst-no-key-found nil t)
	      (progn
		(beginning-of-line)
		(setq curp (point))
		(goto-char (point-max))
		(delete-region curp (point))
		(insert "* No PGP key found in this message.\n"))
	    (progn
	      (setq curp (point))
	      (while (re-search-forward "^pub +[0-9]+/[0-9ABCDEF]+ +199[0-9]/[01][0-9]/[0-3][0-9] +\\(.*\\)\n[^s][^i][^g][^!]" nil t)
					; search for added keys to pubring
					; add them to the list of pubkeys
		(setq pgp-list-keys
		      (cons
		       (cons
			(cons
			 (buffer-substring (match-beginning 1) (match-end 1))
			 1)
			(car pgp-list-keys))
		       (cdr pgp-list-keys)))
		(setq new-key (1+ new-key))
		)
	      (goto-char curp)
	      (while (re-search-forward "^sig! +[0-9ABCDEF]+ +199[0-9]/[01][0-9]/[0-3][0-9] +.*\n" nil t)
					; count the new signatures
		(setq new-sig (1+ new-sig)))
	      (goto-char curp)
					; remove all not pub or sig lines
	      (perform-replace "^" "$$" nil t nil)
	      (goto-char curp)
	      (perform-replace "^\\$\\$\\(pub +[0-9]+/[0-9ABCDEF]+ +199[0-9]/[01][0-9]/[0-3][0-9] +.*\\)$" "\\1" nil t nil)
	      (goto-char curp)
	      (perform-replace "\\$\\$\\(sig! +[0-9ABCDEF]+ +199[0-9]/[01][0-9]/[0-3][0-9] +.*\\)$" "\\1" nil t nil)
	      (goto-char curp)
	      (perform-replace "^\\$\\$.*\n" "" nil t nil)
	      (goto-char (point-max))
	      (delete-backward-char 2)
	      (if (< 0 (+ new-key new-sig))
		  (progn
					; sort the buffer on key ID
		    (sort-regexp-fields
		     nil
		     "^pub +[0-9]+/[0-9ABCDEF]+ +199[0-9]/[01][0-9]/[0-3][0-9] +\\(.*\n\\(sig! +[0-9ABCDEF]+ +199[0-9]/[0-1][0-9]/[0-3][0-9] +.*\n\\)*\\)"
		     "\\1" (point-min) (point-max))
					; remove all duplicate pub lines
		    (goto-char (point-min))
		    (while (re-search-forward "^pub +[0-9]+/[0-9ABCDEF]+ +199[0-9]/[01][0-9]/[0-3][0-9] +.*$" nil t)
		      (setq curp (point))
		      (beginning-of-line)
		      (setq string (buffer-substring (point) curp))
		      (forward-line 1)
		      (while (re-search-forward
			      (concat "^" string "\n") nil t)
			(setq where (point))
			(forward-line -1)
			(delete-region (point) where))
		      (goto-char curp))
					; add a new line before each pub
		    (goto-char (point-min))
		    (forward-line 4)
		    (insert
		     "* Following new key(s) and signature(s) added:\n")
					; reformat the out put
		    (perform-replace "^pub +[0-9]+/[0-9ABCDEF]+ +199[0-9]/[01][0-9]/[0-3][0-9] +.*$" "\n\\&" nil t nil)
		    (goto-char (point-min))
		    (perform-replace "^pub +\\([0-9]+\\)/\\([0-9ABCDEF]+\\) \\(199[0-9]/[0-1][0-9]/[0-3][0-9]\\) \\(.*\\)$" "Key for user ID: \\4\n\\1-bit key, Key ID \\2, created \\3" nil t nil)
		    (goto-char (point-min))
		    (perform-replace "^sig! +\\([0-9ABCDEF]+\\) +\\(199[0-9]/[0-1][0-9]/[0-3][0-9]\\) +\\(.*\\)$" "- signed with key for user ID: \\3\n  key ID \\1, created \\2" nil t nil)
		    (goto-char (point-max))
		    (insert (format
			     "\n %s new key(s) and %s new signature(s) added.\n"
			     (number-to-string new-key)
			     (number-to-string new-sig)))
		    (insert
		     "To sign the new key(s) yourself, run \"pgp -ks\"\n")
		    (insert
		     "To edit the trust information, run \"pgp -ke\"\n")
		    )
		(insert "* No new key or signature found in this message.\n")
		)
	      ))
	  ))
    (set-buffer buf))
;;; Display *PGP output* buffer
  (with-output-to-temp-buffer "*PGP output*"
    (set-buffer standard-output)
    (local-set-key "\C-ca" 'pgp-add-key)
    (local-set-key "\C-cd" 'pgp-decr-mess)
    (insert-buffer "*PGP process*")
    (kill-buffer "*PGP process*")
    (pgp-print-return-message))
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-update-pass          |
;;;| Purpose               : Save the pass phrase associated to a
;;;|                         secret key or wipe it out|
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-general-sign, pgp-sign-encrypt,
;;;|  pgp-decrypt                                     |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-update-pass ()
  "Save the pass phrase associated to a secret key or wipe it out.
If pgp-delete-pass is set to non-nil the pass phrase is wiped out after
each run of pgp, else it is kept during a session of Emacs. There is a
security hole if the pass phrase is in memory of Emacs when it crashes,
it will appear in the core file."

;;; --------------- End function identification ----

  (if pgp-delete-pass
					; delete the pass phrase
      (setq pgp-selected-seckey (cons (car pgp-selected-seckey) nil))
					; save the pass phrase
    (let ((list (cdr pgp-list-keys))
	  (list-res nil))
      (while (not (string= (car pgp-selected-seckey)
			   (car (car list))))
	(setq list-res (append list-res (list (car list))))
	(setq list (cdr list))
	)
      (setq pgp-list-keys
	    (cons (car pgp-list-keys)
		  (append list-res
			  (list pgp-selected-seckey)
			  (cdr list))))
      )))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-print-return-message |
;;;| Purpose               : Display or return message saying how to restore
;;;|                         windows after PGP command|
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     :                           |
;;;| Calling functions    : pgp-show-status           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;Stolen from help.el
(defun pgp-print-return-message ()
  "Display or return message saying how to restore windows after PGP command.
Computes a message and print it."

;;; --------------- End function identification ----

  (and (not (get-buffer-window standard-output))
       (message
	(concat
	 (substitute-command-keys
	  (if (one-window-p t)
	      (if pop-up-windows
		  "Type \\[delete-other-windows] to remove PGP window."
		"Type \\[switch-to-buffer] RET to remove PGP window.")
	    "Type \\[switch-to-buffer-other-window] RET to restore the other window."))
	 (substitute-command-keys
	  "  \\[scroll-other-window] to scroll the PGP.")))))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-decrypt              |
;;;| Purpose               : Decrypt a pgp message    |
;;;| Calling parameters    :                          |
;;;|  buffer: the buffer to read the message from     |
;;;|  begreg, endreg: the region to decrypt           |
;;;| Return value          : t or nil                 |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-wait-process-exit, pgp-update-pass
;;;| Calling functions    : pgp-decr-mess             |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

(defun pgp-decrypt (buffer begreg endreg)
  "Decrypt a pgp message.
The message to decrypt is in buffer between begreg and endreg points, this
allows recursive calls. Return non-nil if the message have been scanned
by pgp."

;;; --------------- End function identification ----

  (save-excursion
    (let ((out-buf (set-buffer "*PGP process*"))
	  (loc-beg pgp-buffer-search-point)
	  (proc))
      (setq inhibit-quit t)
      (setq proc (start-process "pgp-process" out-buf pgp-program "-f"))
      (message "PGP: decrypting message...")
      (process-kill-without-query proc)
      (set-marker (process-mark proc) (point))
      (set-process-filter proc 'pgp-output-filter)
      (set-buffer buffer)
      (sit-for 0 500)
					; Extract the message
      (process-send-region (process-name proc) begreg endreg)
      (if (not (string= (buffer-substring (1- endreg) endreg) "\n")))
					; Add a \n at the end of buffer
      (process-send-string (process-name proc) "")
      (set-buffer out-buf)
      (setq pgp-exit-status
	    (pgp-wait-process-exit proc))
					; Wait for process termination
      (cond ((= pgp-exit-status 0)
;;;With filter mode, pgp doen't exit an error if the standard input contains
;;;no pgp-able information, so I cannot run recursive call	     
;;;     (goto-char (process-mark proc))
;;;     (insert (concat "\n" pgp-strconst-recur-sep "\n"
;;;		     (number-to-string loc-beg) ":"
;;;		     (number-to-string (point-max)) "\n"))
;;;     (set-marker (process-mark proc) (point))
;;;     (pgp-decrypt (current-buffer) loc-beg (point-max))
	     (if pgp-selected-seckey
		 (pgp-update-pass))
	     t)
					; There is no recursive decrypt
;;;    ((= pgp-exit-status 3)
;;;     t)
					; Only unreadable message, should
					; show status anyway
	    ((= pgp-exit-status 31)
	     t)
					; Message altered during
					;transportation
	    ((= pgp-exit-status 1)
	     t)
	    (t nil))
    )))

;;; --------------- Function identification ----------
;;;| Function name         : pgp-decr-mess            |
;;;| Purpose               : Decrypt an Emacs buffer  |
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-list-pubring, pgp-list-secring,
;;;|  pgp-create-output-buffer, pgp-decrypt, pgp-show-status
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-decr-mess ()
  "Decrypt an Emacs buffer.
It can be called from RMAIL buffer or from the 2 output buffers."
  (interactive)

;;; --------------- End function identification ----

  (let ((buf (current-buffer)))
    (save-excursion
      
      (if (and (pgp-list-pubring) (pgp-list-secring))
	  (progn
	    (set-buffer (pgp-create-output-buffer))
	    (setq pgp-buffer-search-point (point-min))
	    (set-buffer buf)
	    (if (pgp-decrypt (current-buffer) (point-min) (point-max))
		(pgp-show-status "d")

	      (progn
		(message "PGP: decrypting aborted.")
		(ding)
		))))
      ))
  )

;;; --------------- Function identification ----------
;;;| Function name         : pgp-add-key              |
;;;| Purpose               : Extract key blocs and update plublic keyring
;;;| Calling parameters    :                          |
;;;| Return value          : non significant          |
;;;--------------------------------------------------

;;; --------------- Cross calls ----------------------
;;;| Called functions     : pgp-list-pubring, pgp-create-output-buffer,
;;;|  pgp-wait-process-exit, pgp-show-status          |
;;;| Calling functions    :                           |
;;;--------------------------------------------------

;;; --------------- Function definition ------------

;;;###autoload
(defun pgp-add-key ()
  "Extract key blocs and update plublic keyring.
It also update the list of public keys accordingly. It can be called from
RMAIL buffer or from the 2 output buffer."
  (interactive)

;;; --------------- End function identification ----

  (let ((buf (current-buffer))
	(out-buf)
	(proc))
    (save-excursion
      (if (pgp-list-pubring)
	  (progn
	    (setq out-buf (pgp-create-output-buffer))
	    (setq inhibit-quit t)
	    (setq proc (start-process "pgp-process"
				      out-buf pgp-program "-kaf"))
	    (message "PGP: adding key...")
	    (process-kill-without-query proc)
	    (setq pgp-buffer-search-point (point-min))
	    (set-buffer buf)
					; goto mail buffer
					; extract the message
	    (sit-for 0 500)
	    (process-send-region (process-name proc) (point-min) (point-max))
	    (if (not (string=
		      (buffer-substring (1- (point-max)) (point-max))
		      "\n"))
		(process-send-string (process-name proc) "\n")
					; Add a \n at the end of buffer
	      )
	    (process-send-string (process-name proc) "")
					; send the message to pgp, add ^D
	    
	    (set-buffer out-buf)
	    (setq pgp-exit-status
		  (pgp-wait-process-exit proc))
					; Wait for process termination
	    (cond ((or (= pgp-exit-status 0)
					; No key found
		       (= pgp-exit-status 12))
		   (pgp-show-status "a")
		   t)
		  (t
		   (message "PGP: add key aborted.")
		   (ding)
		   nil))))
      (set-buffer buf))))
