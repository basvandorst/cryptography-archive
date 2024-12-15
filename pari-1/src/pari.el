;; This is the interface for pari under emacs
;; The main commands in this file are
;; M-x gp      Opens a buffer for interaction with gp and then starts gp.
;; C-u M-x gp  Like M-x gp, but prompts for command line arguments.
;; M-x gpman   Displays the gp-pari manual using any dvi preview program.

;; All functions of gp are preserved.

;; This version by David Carlisle (JANET: carlisle@uk.ac.man.cs).
;; The original pari.el was written by Annette Hoffman.


;; Version 2.13 (06-July-1993)

;; See  pari.txt  for more details.


(provide 'gp)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; The following five constants (aka variables !) should be
;; set for each site.

(defconst gp-chap3 "~pari/PARI/doc/usersch3.tex"
  "The TeX source for chapter 3 of the PARI-GP manual")

(defconst gp-file-name "/usr/local/bin/gp"
  "The file name of the gp executable file")

(defconst gp-man-dvi "~pari/PARI/SPARCV8/doc/users.dvi" 
 "dvi version of the manual")

(defconst  gp-menu "~pari/PARI/pari.menu"
   "menu file")

(defconst gp-dvi-preview "xdvi -s 3"
;; (defconst gp-dvi-preview "texsun"
  "dvi previewer (and options)")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Individual users may want to re-set some of the variables in this section
;; in a gp-mode-hook in their .emacs file.

;; See pari.txt for an example of a gp-mode-hook.

(defvar gp-stack-size "4000000"
  "Default stack size: passed to the progam gp.")

(defvar gp-buffer-size "30000"
  "Default buffer size: passed to the progam gp.")

(defvar gp-prime-limit "500000"
  "Default prime limit: passed to the progam gp.")

(defvar gp-prompt-for-args nil
  "A non-nil value makes M-x gp act like C-u M-x gp, 
   ie prompt for the command line arguments.")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(setq gp-temp-file (make-temp-name "/usr/tmp/gp_"))

(defvar gp-prompt-pattern
  "---- (type return to continue) ----\\|\\?[\C-j\t ]*"
  "Regexp used to match gp prompts.
   can be set with gp-set-prompt (bound to M-\\ p)")

(defconst gp-c-array (make-vector 509 0)
  "obarray used fo completing gp command names")


(defvar gp-map (make-sparse-keymap)
  "Local keymap used in buffer *PARI*.")

(define-key gp-map "\C-m"    'gp-send-input)
(define-key gp-map "\M-\C-m" 'gp-copy-input)
(define-key gp-map "\M-\t"   'gp-complete)
(define-key gp-map "\M-\\p"  'gp-set-prompt)
(define-key gp-map "\M-\\t"  'gp-meta-t)
(define-key gp-map "\M-\\d"  'gp-meta-d)

(define-key gp-map "\M-\\r"  'gp-meta-r)
(define-key gp-map "\M-\\w"  'gp-meta-w)
(define-key gp-map "\M-\\v"  'gp-meta-v)
(define-key gp-map "\M-\\x"  'gp-meta-x)
(define-key gp-map "\M-\\s"  'gp-meta-s)
(define-key gp-map "\M-\\a"  'gp-meta-a)
(define-key gp-map "\M-\\b"  'gp-meta-b)
(define-key gp-map "\M-\\m"  'gp-meta-m)
(define-key gp-map "\M-\\k"  'gp-meta-k)
(define-key gp-map "\M-\\q"  'gp-meta-q)
(define-key gp-map "\M-?"    'gp-get-man-entry)
(define-key gp-map "\M-\\c"  'gp-menu)    
(define-key gp-map "\M-\\\\"  'gp-break-long-line)
(define-key gp-map "\C-c"    'gp-interrupt)

(defvar gp-process nil "")
(defvar gp-man-process nil "")

(defun gp (flag)
  "
   Open a buffer and a window for the execution of gp.

   The following bindings are available:
   \\{gp-map}

  The variables
  gp-file-name gp-stack-size gp-buffer-size gp-prime-limit
  determine the command line that starts gp.
  To override the default settings, give gp a prefix argument.
  C-u M-x gp ."
  (interactive "P")
;; Create buffer *PARI* and make it become the current buffer.
  (switch-to-buffer "*PARI*") 
  (goto-char (point-max))
  (if (and (processp gp-process) 
           (eq (intern "run") (process-status gp-process)))
;; If gp is already running, do  nothing.
    nil
;; Else start up gp in the buffer.
    (kill-all-local-variables)
    (setq major-mode 'gp)
    (setq mode-name "GP")
;; Set up user preferences.
    (run-hooks 'gp-mode-hook)

;; Make gp-map the local map of buffer *PARI*.
    (use-local-map gp-map)      
    (setq mode-line-process '(": %s"))

;; Now we can make the default completion array, must be here, as
;; gp-menu may be set in the user's hook.
    (gp-completion-file gp-menu)

;; Form the command line string.
    (let* (
      (flag (or flag gp-prompt-for-args))
      (gp-command
      (concat
        (gp-read-input "Gp executable ?" gp-file-name "" flag)
        (gp-read-input "Stack size ?" gp-stack-size " -s " flag)
        (gp-read-input "Buffer size ?" gp-buffer-size " -b " flag)
        (gp-read-input "Prime limit ?" gp-prime-limit " -p " flag))))
;; Insert the command line string into the *PARI* buffer.
;; (This is just for reference.)
      (insert (concat "\n" gp-command "\n"))
;; Start gp.
      (let ((gp-begin (point)))
        (setq gp-process
          (start-process 
            "pari" "*PARI*" shell-file-name "-c" 
              (concat "stty nl; exec " gp-command)))
;; Get the version number from the banner.
        (gp-wait-for-output)
        (save-excursion
        (re-search-backward "Version  *\\([.0-9]*\\)" gp-begin)
        (setq gp-version (buffer-substring (match-beginning 1) (match-end 1))))))
;; Clean up when the gp process has finished.
      (set-process-sentinel gp-process  'gp-sentinel)))

(defun gp-read-input (prompt default sep flag)
" If flag is non-nil, reads string then if string is \"\" uses default.
  If flag is nil then string is the default.
  If resulting string is not \"\" prepends sep.
  As a special case, if string is a space, return \"\"."
  (let ((string 
    (if flag
;; If flag is non-nil prompt for input from mini-buffer.
      (read-input 
        (concat prompt " (Default "default") "))
;; Else use the default string.
        default)))
    (if (equal string "")
      (if (equal default "") 
;; If sting and default both "": 
         ""
;; If string "" and default is non empty:
         (concat sep default))
      (if (equal string " ")
;; If string is a space:
        ""
;; If string is non empty:
        (concat sep string)))))

(defun gp-sentinel (proc msg)
  "Sentinel for the gp-process in buffer *PARI*."
    (if (get-buffer "*gp-help*") (kill-buffer "*gp-help*"))
    (let ((b (get-file-buffer gp-chap3))) (if b (kill-buffer b)))
    (let ((b (get-file-buffer gp-menu)))  (if b (kill-buffer b)))
    (if (get-buffer "*PARI*")
      (progn
        (set-buffer "*PARI*")
        (goto-char (point-max))
        (insert msg)
        (delete-windows-on "*PARI*")))
    (if (file-exists-p gp-temp-file) 
       (progn (delete-file gp-temp-file) 
              (message "Removing %s" gp-temp-file )))
  (setq gp-process nil))

(defun gpman()
  "Start up xdvi with the gp manual."
  (interactive)
;; Run gp-mode-hook in case it specifies a different
;; version of the manual.
  (run-hooks 'gp-mode-hook)
  (set-buffer (get-buffer-create "*GP-MAN*"))
  (if gp-man-process
     nil
;; Start xdvi.
    (message (concat "Starting " gp-dvi-preview " " gp-man-dvi))
    (setq gp-man-process
      (start-process "gp-man" "*GP-MAN*"
          shell-file-name
          "-c" (concat "exec " gp-dvi-preview " " gp-man-dvi)))
    (set-process-sentinel gp-man-process 'gp-man-sentinel)))

(defun gp-man-sentinel (proc msg)
  "Sentinel for the gp-man-process in buffer *GP-MAN*."
  (let ((buf (process-buffer proc)))
    (if buf (kill-buffer buf)))
  (message (concat "gpman: " msg))
  (setq gp-man-process nil))

(defun gp-copy-input()
  "Copy expression around point to the end of the buffer.
   (Unless this is already the last expression.)"
  (interactive)
;; Go back to the end of prompt, and record that point.
;; If a line contains more than one prompt string, use the FIRST.
;; This is so that ? ?cos works. (ie gives the help for cos).
;; We can not insist on prompt at the beginning of a line (ie put
;; ^ in gp-prompt-pattern) because of the output from print1()
  (re-search-backward gp-prompt-pattern)
  (let ((p (match-end 0)))
    (beginning-of-line)
    (re-search-forward gp-prompt-pattern)
    (setq gp-input-start (point))
;; Go past the last prompt on this line before looking for end of expression.
    (goto-char p))
;; Check if we are already at the last expression.
  (let ((nlast (re-search-forward (concat "[\n ]*\\("
       gp-prompt-pattern
       "\\|^%[0-9]+ = \\|^ *\\*\\*\\*\\|^ *unused characters\\|^ *time[ r]\\)")
             (point-max) t)))
;; Find the limit of the expression.
;;   This is the end of buffer, the prompt, or an error message.
  (let ((limit (if nlast (match-beginning 0) (point-max))))
  (goto-char gp-input-start)
;; End of expression is a line ending with } if it starts with {,
;;   otherwise a line not ending with \.
  (let ((end-expression (if (looking-at " *{") "}$" "[^\\]$")))
;; Look for the end of expression, as far as the limit computed above.
  (setq gp-complete-expression  (re-search-forward end-expression limit 1))
  (setq gp-input-end (point))
;; If not already the last expression copy to the end of the buffer.
  (if nlast 
   (progn
     (goto-char (point-max))
     (insert (buffer-substring gp-input-start gp-input-end))
     (if gp-complete-expression
       nil
       (ding)
       (message "Incomplete expression."))))))))

(defun gp-send-input ()
  "Send input to gp. Does not send incomplete expressions
   ie those starting with {, without a matching }, or those
   ending with \\ .
   Use a temporary file (and \\r ) for large expressions"
  (interactive)
;; gp-copy-input does all the work!
  (gp-copy-input)
  (insert "\n")
  (if gp-complete-expression
;; If it is a complete expression do this:
    (progn
      (if (> (- gp-input-end gp-input-start) 255)
;; If large expression, use a temporary file.
        (progn 
          (write-region gp-input-start gp-input-end gp-temp-file)
          (process-send-string gp-process (concat "\\r "gp-temp-file"\n")))
;; Else use process-send-region.
        (process-send-region gp-process gp-input-start gp-input-end)
        (process-send-string gp-process "\n"))
      (set-marker (process-mark gp-process) (point)))
;; Else do this:
    (message "Incomplete expression: Not sent to gp.")))

(defun gp-interrupt ()
  "Interrupt gp.
   This is identical to interrupt-shell-subjob in shell-mode."
  (interactive)
  (interrupt-process nil t))

(defun gp-set-prompt (p)
  "Set new gp prompt (and tell emacs that you have done so).
   Do not put spaces in the argument, or emacs and gp will
   have a different idea about what the prompt is."
  (interactive "sNew prompt: ")
;; gp-prompt-pattern matches:
;; (New prompt plus any following white space) OR (Old pattern).
  (setq gp-prompt-pattern 
    (concat (regexp-quote p) "[\C-j\t ]*\\|" gp-prompt-pattern))
  (set-buffer "*PARI*")
  (goto-char (point-max))
;; Tell gp about the change too!
  (insert (concat "\\prompt="p))
  (gp-send-input))

(defun gp-replace (a b)
  "Replace the regexp a by the string b everywhere in the current buffer"
  (goto-char (point-min))
  (while (re-search-forward a (point-max) t)
    (replace-match b t t)))

(defun gp-get-man-entry (fn)
  "Obtains the description of fn from chapter 3 of the manual.
  Strips off some (not all) of the TeX syntax, and displays the result
  in a new window.
  If there is no entry for fn in the manual, sends ?fn to gp.
  If a definition for fn is found, adds fn to the array of possible
  completions"
  (interactive
    (list  (let* (
             (word 
;; get the word before point into word
              (save-excursion
                 (forward-word -1)
                 (downcase (buffer-substring (point) 
                   (progn  (forward-word 1) (point))))))
;; get the argument from the minibuffer
             (arg
               (completing-read 
                 (concat "Function" 
                   (if (intern-soft word gp-c-array)
;; If the word before point is a gp function, offer it as default.
                     (concat " (Default " word ")" )) ": ")
;; use gp-c-array as the completion array
                  gp-c-array)))
      (if (equal arg "")
;; If the argument supplied is "", and word is a gp symbol, use it as default.
;; (Do not use "" as fn in anycase, so otherwise use " ", which will not
;; produce a help window.)
        (if (intern-soft word gp-c-array) word " ") 
;; Else use the arg.
        arg))))
;; end of interactive call
;; Stop TeX-mode being loaded for gp-chap3.
  (let ((auto-mode-alist nil))
    (set-buffer (find-file-noselect gp-chap3)))
;; Fix up one or two special cases, or 
;; regexp-quote the argument.
  (let ((qfn (cond
           ((equal fn "\\" ) "\\\\backslash")
           ((equal fn "^" ) "\\\\hat{}")
           ((equal fn "!" ) "fact")
           ((equal fn "~" ) "trans") 
           ((equal fn "_" ) "conj")
           ((equal fn "-" ) "\\+")
           ((equal fn "%" ) "\\\\%")
           ((equal fn "min" ) "max")
           ((equal fn "log" ) "ln")
           ((equal fn "det2" ) "det")
           ((or (equal fn "<=" )(equal fn "<" )(equal fn ">=" )
            (equal fn ">" )(equal fn "==" )(equal fn "!=" )
            (equal fn "||" )(equal fn "&&" )) 
                      "comparison and \\\\ref{boolean operators}")
           ((regexp-quote fn)))))
;; Find the entry.
  (goto-char (point-min))
;; Entry starts with \subsec ... fn
  (if (re-search-forward 
     (concat "\\(subsec[\\\\{ref]*[\\${]\\)" qfn "[}\\$]") (point-max) t)
;; If There is an entry in the manual do this:
  (progn
    (goto-char (match-end 1))
    (let ((copy (buffer-substring (point) 
;; Entry ends with "The library" or the next (sub-)section.
          (progn (re-search-forward "[tT]he library\\|\\\\[sub]*sec" 
                    (point-max) t) 
                 (match-beginning 0))))
          (wind (selected-window)))
;; Copy the entry to the help buffer.
    (switch-to-buffer-other-window (get-buffer-create "*gp-help*"))
    (erase-buffer)
    (insert copy)
;; Strip off some of the TeX. Note the idea is to leave enough
;; pseudo-TeX so that the entry is understandable. Thus want to
;; leave:  a^2,  x \over y, etc.
    (gp-replace "\\$-" "-")
    (gp-replace "\\$" " " )
    (gp-replace "\\\\backslash" "\\")
    (gp-replace "\\\\hat" "^")
    (gp-replace "\\\\vers" gp-version)
    (gp-replace
"\\\\smallskip\\|\\\\sref{[ a-z]*}\\|\\\\bf\\|\
\\\\ref\\|\\\\Bbb\\|\\\\text\\|\\\\tt\\|{\\|}"
      "")
    (goto-char (point-min))
    (select-window wind)))
;; Else there is no entry in the manual. So send ?fn to gp.
  (set-buffer "*PARI*")
;;; (downcase fn) as ?COS does not work gp bug ??
  (gp-meta-command (concat "?" (downcase fn)))))
;; Addition at version 2.10
;; If the result is a gp function make sure that it is in the
;; completion array.
  (set-buffer "*gp-help*")
  (if (looking-at "Unknown function")
;; If the fuction is not known hide the help window.
       (progn (bury-buffer (get-buffer "*gp-help*"))
              (delete-windows-on "*gp-help*") 
              (message "Unknown function: %s" fn))
;; Else let the completion system know about the fuction name.
       (gp-add-symbol fn)))

(defun gp-meta-command (command)
  "Send command to gp, and display output in help buffer"
  (save-excursion
  (goto-char (point-max))
;; Make sure that gp sends the text to the end of the buffer, so we
;; can move it  to the help buffer.
  (set-marker (process-mark gp-process) (point))
  (let ((temp (point)) (wind (selected-window)))
;; Send the meta command to gp.
  (process-send-string gp-process (concat command "\n"))
;; Wait for the gp-prompt to be sent.
  (gp-wait-for-output)
;; Display the output in the help buffer.
  (let ((copy (buffer-substring temp (point-max))))
  (delete-region temp (point-max))
  (switch-to-buffer-other-window (get-buffer-create "*gp-help*"))
  (erase-buffer)
  (insert copy)
  (beginning-of-line)
  (delete-region (point) (point-max))
  (goto-char (point-min))
  (select-window wind)))))

(defun gp-wait-for-output ()
  "Hang around until the prompt appears."
  (let ((ndone t))
  (while ndone 
  (accept-process-output gp-process)
  (let ((p (point))) 
    (beginning-of-line)
    (if (or (not (and (processp gp-process) 
             (eq 'run (process-status gp-process))))
             (looking-at gp-prompt-pattern))
;; If gp is not running, or the prompt has appeared, stop.
      (progn (message "done") (setq ndone nil))
;; Else wait a bit longer.
      (message "Waiting for gp output ..."))
    (goto-char p)))))

(defun gp-meta-d ()
  "Sends \\d to gp, then displays output in the help buffer.
  Prints the gp defaults."
  (interactive)
  (gp-meta-command "\\d"))

(defun gp-meta-t ()
  "Sends \\t to gp, then displays output in the help buffer.
  Prints the longword format of PARI types."
  (interactive)
  (gp-meta-command "\\t"))

(defun gp-meta-r (file)
  "Sends a \\r <file name> comand to gp.
   Reads in gp commands from a file."
  (interactive "fRead from file: ")
  (goto-char (point-max))
  (insert (concat "\\r " (expand-file-name file)))
  (gp-send-input))

(defun gp-meta-w (file num)
  "Sends a \\w<num> <file name> comand to gp.
  Writes gp object %<num> to <file name>."
  (interactive "FWrite to file: \nsObject number %%")
  (goto-char (point-max))
  (insert (concat "\\w"num" " (expand-file-name file)))
  (gp-send-input))

(defun gp-meta-x ()
  "Sends \\x to gp, then displays output in the help buffer.
  Prints tree of addresses and contents of last object."
  (interactive)
  (gp-meta-command "\\x"))

(defun gp-meta-v ()
  "Sends \\v to gp, then displays output in the help buffer.
  Prints the version number of this implementation of pari-gp."
  (interactive)
  (gp-meta-command "\\v"))

(defun gp-meta-s (num)
  "Sends \\s or \\s(num) to gp, then displays output in the help buffer.
  Prints the state of the pari stack."
  (interactive "sNumber of longwords (default 0) ")
  (if (equal num "")
    (gp-meta-command "\\s")
    (gp-meta-command (concat "\\s(" num ")" ))))

(defun gp-meta-a (num)
  "Sends \\a or \\a<num> to gp, then displays output in the help buffer.
  Prints object %<num> in raw format."
  (interactive "sPrint object (default last) %%")
  (if (equal num "")
    (gp-meta-command "\\a")
    (gp-meta-command (concat "\\a" num))))

(defun gp-meta-b (num)
  "Sends \\b or \\b<num> to gp, then displays output in the help buffer.
  Prints object %<num> in pretty format."
  (interactive "sPrint object (default last) %%")
  (if (equal num "")
    (gp-meta-command "\\b")
    (gp-meta-command (concat "\\b" num))))

(defun gp-meta-m (num)
  "Sends \\m or \\m<num> to gp, then displays output in the help buffer.
  Prints object %<num> in prettymatrix format."
  (interactive "sPrint object (default last) %%")
  (if (equal num "")
    (gp-meta-command "\\m")
    (gp-meta-command (concat "\\m" num))))

(defun gp-meta-k ()
  "Sends \\k to gp.
  Prompts for confirmation before 
  re-initialising gp and clearing the buffer."
  (interactive) 
  (if (y-or-n-p "Re-initialise gp ? ") 
    (progn
      (set-buffer "*PARI*")
      (goto-char (point-max))
      (insert "\\k\n")
      (set-marker (process-mark gp-process) (point))
      (if (y-or-n-p "Clear *PARI* buffer ? ")
         (erase-buffer))
     (process-send-string gp-process "\\k\n")))
  (message ""))

(defun gp-meta-q ()
  "Sends \\q to gp.
  Prompts for confirmation before quiting."
  (interactive) 
  (if (y-or-n-p "Quit gp ? ") 
    (progn
     (set-buffer "*PARI*")
     (goto-char (point-max))
     (process-send-string gp-process "\\q\n")))
  (message ""))

(defun gp-break-long-line ()
  "gp will not accept lines longer than 256.
   gp-break-long-line breaks current line 
   inserting \\ every (screen-width)-5 chars."
  (interactive)
  (let ((length (min (- (screen-width) 5) 250)))
  (move-to-column length)
  (while (not (looking-at "$"))
    (insert "\\\n")
    (move-to-column length))))

;;; gp completion functions.

(defun gp-completion-file (file)
  "Takes a file in the format of pari.menu, and adds all the commands
  listed  to the obarray used for completion. If used interactively,
  prompts for the filename in the minibuffer
  The file must have at least one comment line, starting with #, All
  lines before the first comment line are IGNORED."
  (interactive "fFile of command names: ")
  (save-excursion
    (set-buffer (find-file-noselect file))
    (goto-char (point-min))
    (re-search-forward "#")
    (while (not (eobp))
      (forward-line 1)
      (if (looking-at "#")
        nil
;; else
       (gp-add-symbol (buffer-substring
       (point)
       (progn (end-of-line) (point))))))))

(defun gp-completion-list (&rest args)
  "The arguments are taken as a list of strings, each string is stored
  as a symbol in the obarray used for completion.
  So (gp-completion-list \"foo\" \"bar\" \"baz\") in the gp-mode-hook will
  let the completion functions 'know' these commands."
  (mapcar 'gp-add-symbol args))

(defun gp-add-symbol  (name)
  "Add a name to the obarray"
  (make-symbol name)
  (intern (downcase name) gp-c-array))

(defun gp-complete ( )
  "Attempts to complete a partially typed command in the *PARI*
  buffer. Displays possible completions in the help buffer if no
  unique completion can be done."
  (interactive)
  (if (not(= (preceding-char) ?\\ )) (forward-word -1))
  (if (= (preceding-char) ?\\ ) (forward-char -1))
    (let* ((c-begin (point))
          (word (downcase (buffer-substring c-begin 
				  (progn   (forward-word 1) (point)))))
          (comp (try-completion word gp-c-array)))
;; The 3 possible outcomes of completion:
     (cond 
;; (1)   Already complete.
        ((eq comp t) (if (get-buffer "*gp-help*") 
                        (delete-windows-on "*gp-help*"))
                      (message "%s is complete" word))
;; (2)   Not found. (It may be a valid gp function, unknown to the
;;      completion system)
        ((eq comp nil) (message "No Match found"))
;; (3)  Some extension to the word possible.
        (t
          (if (not(string= comp word))
;;     Add any unique extension to the *PARI* buffer.

           (progn (insert comp) 
           (delete-region (+ c-begin (length word)) 
             (+ c-begin (* 2(length word))))))
          (if (eq t (try-completion comp gp-c-array))
;;     If the extended word is definitely complete, remove the help window
             (progn 
               (if (get-buffer "*gp-help*") (delete-windows-on "*gp-help*"))
               (message "%s is complete" comp))
;;     else display all possible completions.
          (with-output-to-temp-buffer "*gp-help*"
           (display-completion-list (all-completions word gp-c-array ))))))))

;; Do not do completion on \prompt, as the user is not supposed to
;; change the prompt, except via M-\ p
(gp-completion-list "\\precision" "\\serieslength" "\\format")

;;; The gp help mode.

(defun gp-menu ()
  "Major-mode for the gp menu buffer.
The available commands are
\\{gp-menu-map}"
  (interactive)
  (find-file-other-window gp-menu)
  (setq buffer-read-only t)
  (kill-all-local-variables)
  (setq major-mode 'gp-menu)
  (setq mode-name "GP MENU")
  (use-local-map gp-menu-map)
  (gp-menu-main))

(defun gp-menu-info ()
  (message "SPC=next DEL=previous RET=select m=main-menu q=quit s=scroll-help"))

(defun gp-menu-next ()
  "Move down one line of the gp help menu. (Go to top if at the end.)"
  (interactive)
  (gp-menu-info)
  (forward-line 1)
  (if (eobp) 
    (progn (ding) (goto-char (point-min)))))

(defun gp-menu-previous ()
  "Move up one line of the gp help menu. (Go to bottom if at the top.)"
  (interactive)
  (gp-menu-info)
  (if (bobp) 
    (progn (ding) (goto-char (point-max)) (beginning-of-line)) 
    (forward-line -1)))

(defun gp-menu-quit ()
  "Switch the *PARI* buffer if it exists, or (other-buffer) if it does not."
  (interactive)
  (let ((w (get-buffer-window "*PARI*"))
        (b (get-buffer "*PARI*")) )
    (cond
     (w (progn (delete-window)(select-window w)))
     (b (switch-to-buffer b))
     (t (switch-to-buffer (other-buffer))))))

(defun gp-menu-select ()
  "Select a subject from the main menu, or a manual entry from a subject menu."
  (interactive)
  (if main-menu 
;; RET in main menu.
    (progn
      (setq main-menu nil)
      (widen)
      (beginning-of-line)
      (let ((sect (buffer-substring 
           (point) (progn (end-of-line) (point)))))
         (narrow-to-region
           (progn 
             (re-search-forward (concat "^###" sect))
             (forward-line 1) (point))
           (progn (re-search-forward "\C-j###" ) (match-beginning 0))))
      (goto-char (point-min)))
;; RET in subject menu.
    (beginning-of-line)
    (gp-get-man-entry (buffer-substring
      (point)
      (progn (end-of-line) (point)))))
  (gp-menu-info))

(defun gp-menu-main ()
  "Display the main menu."
  (interactive)
  (gp-menu-info)
  (widen)
  (goto-char (point-min))
  (narrow-to-region (point)
    (progn (re-search-forward "\C-j###") (match-beginning 0)))
  (goto-char (point-min))
  (setq done nil)
  (setq main-menu t))

(defun gp-menu-scroll ()
  "Scroll the gp help window if it is visible"
  (interactive)
  (gp-menu-info)
  (if (get-buffer-window "*gp-help*") 
    (let ((wind (selected-window)))
      (switch-to-buffer-other-window  "*gp-help*")
      (if (pos-visible-in-window-p (point-max))
         (goto-char (point-min))
         (scroll-up))
      (select-window wind))))
     

(defvar gp-menu-map (make-sparse-keymap)
  "Local keymap used in gp menu buffer.")
      
(define-key gp-menu-map " "    'gp-menu-next)
(define-key gp-menu-map "\C-?" 'gp-menu-previous)
(define-key gp-menu-map "\C-m" 'gp-menu-select)
(define-key gp-menu-map "q"    'gp-menu-quit)
(define-key gp-menu-map "m"    'gp-menu-main)
(define-key gp-menu-map "s"    'gp-menu-scroll)

