;;; Compaction, compression and encryption for GNU Emacs
;;; Copyright (C) 1988, 1989, 1990 Kyle E. Jones
;;;
;;; Converted to DES encryption/decryption by Uri Blumenthal
;;; Copyright (C) 1991 IBM T. J. Watson Research Center
;;;
;;; Please note, that this macros set requires DES package
;;; to be installed on your machine, especially executable
;;; file "descrypt" to be found somewhere in the PATH.
;;; Of course, DES is freely available both inside
;;; of USA and outside of it:
;;;
;;;	Folks inside of USA - please use one of the
;;;     following hosts:
;;;		gatekeeper.dec.com: /pub/ka9q/des
;;;             <anywhere> comp.sources.unix/Volume10 file "des"
;;;             <use your skills :->
;;;
;;;     For you foreigners:
;;;             <lots of FTP sites carrying DES code>
;;;             kth.se:/src/RPC/rpc4.0/secure_rpc/des
;;;             marlin.jcu.edu.au:/pub/des
;;;             chalmers.se:/pub/des
;;;
;;;
;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 1, or (at your option)
;;; any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; A copy of the GNU General Public License can be obtained from this
;;; program's author (send electronic mail to kyle@cs.odu.edu) or from
;;; the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
;;; 02139, USA.
;;;
;;; Send bug reports to kyle@cs.odu.edu.

;; To use this package, put it in a file called "descrypt.el" in a Lisp
;; directory that Emacs knows about, byte-compile it, and put the line:
;;    (require 'descrypt)
;; in your .emacs file or in the file default.el in the "lisp" directory
;; of the Emacs distribution.  Don't bother trying to autoload this file;
;; this package uses a find-file hook and thus should be loaded the
;; first time you visit any sort of file.
;;
;; The basic purpose of this package of Lisp functions is to automatically
;; recognize encrypted, compacted or compressed files when they are first
;; visited and decode the file's BUFFER before it is presented to the user.
;; The file itself is unchanged.  When the buffer is subsequently saved to
;; disk, a hook function re-encodes the buffer before the actual disk write
;; takes place.
;;
;; This package recognizes compacted and compressed files by a magic number at
;; the beginning of these files, but a heuristic is used to detect encrypted
;; files.  If you are asked for an encryption key for a file that is in fact
;; not encrypted, just hit RET and the file will be accepted as is, and the
;; crypt minor mode will not be entered.

(provide 'descrypt)

(defvar auto-decode-buffer t
  "*Non-nil value means that the buffers associated with encoded files will
be decoded automatically, without requesting confirmation from the user.
Nil means to ask before doing the decoding.")

(defvar buffer-save-encrypted nil
  "*Non-nil means that when this buffer is saved it will be written out
encrypted, as with the UNIX crypt(1) command.  Automatically local to all
buffers.")
(make-variable-buffer-local 'buffer-save-encrypted)

(defvar buffer-save-compacted nil
  "*Non-nil means that when this buffer is saved it will be written out
compacted, as with the UNIX compact(1) command.  Automatically local to all
buffers.")
(make-variable-buffer-local 'buffer-save-compacted)

(defvar buffer-save-compressed nil
  "*Non-nil means that when this buffer is saved it will be written out
compressed, as with the UNIX compress(1) command.  Automatically local to all
buffers.")
(make-variable-buffer-local 'buffer-save-compressed)

(defvar buffer-encryption-key nil
  "*Key to use when encrypting the current buffer, prior to saving it.
Automatically local to all buffers.")
(make-variable-buffer-local 'buffer-encryption-key)

(defvar buffer-des-encryption-ascii nil
  "*Non-nil means that encrypted file should be produced as a sequence
of ASCII lines with a single 64-bit hexadecimal number prepended by 0x on
each line. In decryption mode - only such lines are accepted as input. Any
line with other information is ignored and doesn't affect the process.")
(make-variable-buffer-local 'buffer-des-encryption-ascii)

(defconst compact-magic-regexp "\377\037"
  "Regexp that matches the magic number at the beginning of files created
by the compact(1) command.")

(defconst compress-magic-regexp "\037\235\220"
  "Regexp that matches the magic number at the beginning of files created
by the compress(1) command.")

;; Encrypted files have no magic number, so we have to hack a way of
;; determining which new buffers start in crypt mode.  The current setup is
;; that we use only buffers that have a non-ASCII character very close to
;; beginning of buffer and that do NOT match crypt-magic-regexp-inverse.
;; Currently crypt-magic-regexp-inverse will match Sun OS, 4.x BSD, and
;; Ultrix executable magic numbers, so binaries can still be edited (heh)
;; without headaches.

(defconst crypt-magic-regexp-inverse
  "\\(..\\)?\\([\007\010\013]\001\\|\001[\007\010\013]\\)"
  "Regexp that must NOT match the beginning of an encrypted buffer.")

(defmacro save-point (&rest body)
  "Save value of point, evalutes FORMS and restore value of point.
If the saved value of point is no longer valid go to (point-max).
This macro exists because, save-excursion loses track of point during
some types of deletions."
  (let ((var (make-symbol "saved-point")))
    (list 'let (list (list var '(point)))
	  (list 'unwind-protect
		(cons 'progn body)
		(list 'goto-char var)))))


(defun find-crypt-file-hook ()
  (save-point
    (save-restriction
      (widen)
      (goto-char (point-min))
      (let ((buffer-file-name buffer-file-name)
	    (old-buffer-file-name buffer-file-name)
	    (old-buffer-modified-p (buffer-modified-p))
	    encrypted compressed compacted case-fold-search buffer-read-only)
	;; We can reasonably assume that either compaction or compression will
	;; be used, or neither, but not both.
	(cond ((and (looking-at compact-magic-regexp)
		    (or auto-decode-buffer
			(y-or-n-p (format "Uncompact buffer %s? "
					  (buffer-name)))))
	       (message "Uncompacting %s..." (buffer-name))
	       (compact-buffer (current-buffer) t)
	       ;; We can't actually go into compact mode yet because the major
	       ;; mode may change later on and blow away all local variables
	       ;; (and thus the minor modes).  So we make a note to go into
	       ;; compact mode later.
	       (setq compacted t)
	       ;; here we strip the compacted file's .C extension so that later
	       ;; we can set the buffer's major mode based on this modified
	       ;; name instead of the name with the .C extension.
	       (if (string-match "\\(\\.C\\)$" buffer-file-name)
		   (setq buffer-file-name
			 (substring buffer-file-name 0 (match-beginning 1))))
	       (if (not (input-pending-p))
		   (message "Uncompacting %s... done" (buffer-name))))
	      ((and (looking-at compress-magic-regexp)
		    (or auto-decode-buffer
			(y-or-n-p (format "Uncompress buffer %s? "
					  (buffer-name)))))
	       (message "Uncompressing %s..." (buffer-name))
	       (compress-buffer (current-buffer) t)
	       (setq compressed t)
	       (if (string-match "\\(\\.Z\\)$" buffer-file-name)
		   (setq buffer-file-name
			 (substring buffer-file-name 0 (match-beginning 1))))
	       (if (not (input-pending-p))
		   (message "Uncompressing %s... done" (buffer-name)))))
	;; Now peek at the file and see if it still looks like a binary file.
	;; If so, try the crypt-magic-regexp-inverse against it and if it FAILS
	;; we assume that this is an encrypted buffer.
	(cond ((or (and
		    (not (eobp))
		    (re-search-forward "[\200-\377]" (min (point-max) 15) t)
		    (goto-char (point-min))
		    (not (looking-at crypt-magic-regexp-inverse)))
		   (and
		    (goto-char (point-min))
		    (re-search-forward "^0x[0-9a-f]" (min (point-max) 4) t)))
	       (goto-char (point-min))
;	       (if (and
;		    (goto-char (point-min))
;		    (re-search-forward "^0x[0-9a-f]" (min (point-max) 4) t))
;		   (setq buffer-des-encryption-ascii t))
	       (if (not buffer-encryption-key)
		   (call-interactively 'set-encryption-key))
	       ;; if user did not enter a key, turn off crypt mode.
	       ;; good for binaries that crypt-magic-regexp-inverse
	       ;; doesn't recognize.
	       ;; -- thanx to Paul Dworkin (paul@media-lab.media.mit.edu)
	       (if (equal buffer-encryption-key "")
		   (message "No key given, buffer %s assumed normal."
			    (buffer-name))
		 (message "DES-decrypting %s..." (buffer-name))
		 (des-decrypt-buffer buffer-encryption-key)
;				     buffer-des-encryption-ascii )
		 ;; Tuck the key away for safe keeping since setting the major
		 ;; mode may well blow it away.
		 (setq encrypted buffer-encryption-key)
		 (if (not (input-pending-p))
		     (message "DES-decrypting %s... done" (buffer-name))))))
	;; OK, if any changes have been made to the buffer we need to rerun the
	;; code that does automatic selection of major mode.
	(cond ((or compressed compacted encrypted)
	       (set-auto-mode)
	       (hack-local-variables)
	       ;; Now set our minor modes.
	       (if compressed (compress-mode 1))
	       (if compacted (compact-mode 1))
	       (if encrypted
		   (progn (crypt-mode 1)
			  (setq buffer-encryption-key encrypted)))
	       ;; Restore buffer file name now, so that lock file entry is
	       ;; removed properly.
	       (setq buffer-file-name old-buffer-file-name)
	       ;; Restore buffer modified flag to its previous value.
	       ;; This will also remove the lock file entry for the buffer
	       ;; if the previous value was nil; this is why buffer-file-name
	       ;; had to be manually restored above.
	       (set-buffer-modified-p old-buffer-modified-p)))))))

;; This function should be called ONLY as a write-file hook.
;; Odd things will happen if it is called elsewhere.
(defun write-crypt-file-hook ()
  (cond
   ((or buffer-save-encrypted buffer-save-compacted buffer-save-compressed)
    (save-excursion
      (save-restriction
	(let ((copy-buffer (get-buffer-create " *crypt copy buffer*"))
	      (selective-display selective-display)
	      (buffer-read-only))
	  (copy-to-buffer copy-buffer 1 (1+ (buffer-size)))
	  (narrow-to-region (point) (point))
	  (unwind-protect
	      (progn
		(insert-buffer-substring copy-buffer)
		(kill-buffer copy-buffer)
		;; selective-display non-nil means we must convert carriage
		;; returns to newlines now, and set selective-display
		;; temporarily to nil.
		(cond (selective-display
		       (goto-char (point-min))
		       (subst-char-in-region (point-min) (point-max) ?\r ?\n)
		       (setq selective-display nil)))
		(cond
		 (buffer-save-encrypted
		  (if (null buffer-encryption-key)
		      (error "No encryption key set for buffer %s"
			     (buffer-name)))
;		  (if (null buffer-des-encryption-ascii)
;		      (call-interactively 'set-des-encryption-ascii))
		  (if (not (stringp buffer-encryption-key))
		      (error "Encryption key is not a string"))
		  (message "DES-encrypting %s..." (buffer-name))
		  (des-encrypt-buffer buffer-encryption-key
				      buffer-des-encryption-ascii )))
		(cond
		 ((and buffer-save-compacted buffer-save-compressed)
		  (error "Cannot compact and compress buffer %s"
			 (buffer-name)))
		 (buffer-save-compacted
		  (message "Compacting %s..." (buffer-name))
		  (compact-buffer))
		 (buffer-save-compressed
		  (message "Compressing %s..." (buffer-name))
		  (compress-buffer)))
		(write-region (point-min) (point-max) buffer-file-name nil t)
		(delete-region (point-min) (point-max))
		(set-buffer-modified-p nil)
		;; return t so that basic-save-buffer will
		;; know that the save has already been done.
		t )
	    ;; unwind...
	    ;; If the crypted stuff has already been removed
	    ;; then this is a no-op.
	    (delete-region (point-min) (point-max)))))))))




;;;
;;; DES Encryption and decryption for GNU Emacs
;;; Copyright (C) 1991 Uri Blumenthal,
;;;                    IBM T. J. Watson Research Center
;;;
;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 1, or (at your option)
;;; any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; A copy of the GNU General Public License can be obtained from
;;; the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
;;; 02139, USA.
;;;
;;; Send bug reports to uri@watson.ibm.com

(defun des-encrypt-region (start end key des-ascii)
   "DES-encrypt the text in the region.
>From a program, this function takes three agrs: START, END and KEY.
When called interactively - START and END default to point and mark
\(START being the lesser of two\), KEY is prompted for. ASCII defines
that result should be in ASCII - one 64-bit hexadecimal number per line."
   (interactive
    (progn
      (barf-if-buffer-read-only)
      (list
       (region-beginning)
       (region-end)
       (read-string-no-echo "DES-encrypt region using key: ")
       (setq des-ascii (y-or-n-p "Want to store encrypted data in ASCII format? "))
       )
      )
    )
   (if des-ascii
       (setq com-line (concat "descrypt -e -k " key " -a "))
     (setq com-line (concat "descrypt -e -k " key)))
   (call-process-region start end shell-file-name t t nil "-c"
			com-line))

(defun des-decrypt-region (start end key)
   "DES-decrypt the text in the region.
>From a program, this function takes three agrs: START, END and KEY.
When called interactively - START and END default to point and mark
\(START being the lesser of two\), KEY is prompted for. "
   (interactive
    (progn
      (barf-if-buffer-read-only)
      (list
       (region-beginning)
       (region-end)
       (read-string-no-echo "DES-decrypt region using key: ")
; Sacrifice flexibility of ALWAYS being able to tell the right
; type of buffer to convenience of not be bothered by this
; silly question about ASCII. Let it determine by itself!
;       (setq des-ascii (y-or-n-p "Is encrypted data in ASCII? "))
       )
      )
    )
   (if (and (goto-char (point-min))
	    (re-search-forward "^0x[0-9a-f]" nil t)
	    (goto-char (point-min)))
       (setq com-line (concat "descrypt -d -k " key " -a "))
     (setq com-line (concat "descrypt -d -k " key)))
   (call-process-region start end shell-file-name t t nil "-c"
			com-line))

(defun des-encrypt-buffer (key des-ascii &optional buffer)
  "Using KEY, DES-encrypt BUFFER.
ASCII if non-nil tells that encrypted data should be produced in
ASCII, each line containing one 64-bit hexadecimal number.
BUFFER defaults to the current buffer."
  (interactive
   (progn
     (barf-if-buffer-read-only)
     (list
      (read-string-no-echo "DES-encrypt buffer using key: ")
      (setq des-ascii (y-or-n-p "Want encrypted output in ASCII? "))
      )))
  (or buffer (setq buffer (current-buffer)))
  (save-excursion
    (set-buffer buffer)
    (des-encrypt-region (point-min) (point-max) key des-ascii)))

(defun des-decrypt-buffer (key &optional buffer)
  "Using KEY, DES-decrypt BUFFER.
BUFFER defaults to the current buffer."
  (interactive
   (progn
     (barf-if-buffer-read-only)
     (list
      (read-string-no-echo "DES-decrypt buffer using key: ")
;      (setq des-ascii (y-or-n-p "Is encrypted data in ASCII? "))
      )))
  (or buffer (setq buffer (current-buffer)))
  (save-excursion
    (set-buffer buffer)
    (des-decrypt-region (point-min) (point-max) key)))

(defun des-add-checksum (start end key des-ascii)
   "Calculate DES-based cryptographic checksum of the text in the region
and appends it to that region.
>From a program, this function takes three agrs: START, END and KEY.
When called interactively - START and END default to point and mark
\(START being the lesser of two\), KEY is prompted for. ASCII flag
determines whether encrypted data is in ASCII format as hexadecimal
numbers. "
   (interactive
    (progn
      (barf-if-buffer-read-only)
      (list
       (region-beginning)
       (region-end)
       (read-string-no-echo "DES-checksum region using key: ")
       (setq des-ascii t)
       )))
   (if des-ascii
       (setq com-line (concat "descrypt -c -k " key " -a "))
     (setq com-line (concat "descrypt -c -k " key)))
   (call-process-region start end shell-file-name nil t t "-c"
			com-line))


























































(defun compact-region (start end &optional undo)
  "Compact the text in the region.
>From a program, this function takes three args: START, END and UNDO.
When called interactively START and END default to point and mark
\(START being the lesser of the two).
Prefix arg (or optional second arg non-nil) UNDO means uncompact."
  (interactive "*r\nP")
  (save-point
   (call-process-region start end shell-file-name t t nil "-c"
			(if undo "uncompact" "compact"))
   (cond ((not undo)
	  (goto-char start)
	  (let (case-fold-search)
	    (if (not (looking-at compact-magic-regexp))
		(error "%s failed!" (if undo
					"Uncompaction"
				      "Compaction"))))))))

(defun compact-buffer (&optional buffer undo)
  "Compact BUFFER.
BUFFER defaults to the current buffer.
Prefix arg (or second arg non-nil from a program) UNDO means uncompact."
  (interactive (list (current-buffer) current-prefix-arg))
  (or buffer (setq buffer (current-buffer)))
  (save-excursion
    (set-buffer buffer)
    (compact-region (point-min) (point-max) undo)))




























































(defun compress-region (start end &optional undo)
  "Compress the text in the region.
>From a program, this function takes three args: START, END and UNDO.
When called interactively START and END default to point and mark
\(START being the lesser of the two).
Prefix arg (or optional second arg non-nil) UNDO means uncompress."
  (interactive "*r\nP")
  (save-point
   (call-process-region start end shell-file-name t t nil "-c"
			(if undo "compress -d" "compress"))
   (cond ((not undo)
	  (goto-char start)
	  (let (case-fold-search)
	    (if (not (looking-at compress-magic-regexp))
		(error "%s failed!" (if undo
					"Uncompression"
				      "Compression"))))))))

(defun compress-buffer (&optional buffer undo)
  "Compress BUFFER.
BUFFER defaults to the current buffer.
Prefix arg (or second arg non-nil from a program) UNDO means uncompress."
  (interactive (list (current-buffer) current-prefix-arg))
  (or buffer (setq buffer (current-buffer)))
  (save-excursion
    (set-buffer buffer)
    (compress-region (point-min) (point-max) undo)))




























































(defun set-encryption-key (key &optional buffer)
  "Set the encryption KEY for BUFFER.
KEY should be a string.
BUFFER should be a buffer or the name of one;
it defaults to the current buffer.  If BUFFER is in crypt mode, then it is
also marked as modified, since it needs to be saved with the new key."
  (interactive
   (progn
     (barf-if-buffer-read-only)
     (list
      (read-string-no-echo
       (format "Set encryption key for buffer %s: " (buffer-name))))))
  (or buffer (setq buffer (current-buffer)))
  (save-excursion
    (set-buffer buffer)
    (if (equal key buffer-encryption-key)
	(message "Key is identical to original, no change.")
      (setq buffer-encryption-key key)
      ;; don't touch the modify flag unless we're in crypt-mode.
      (if buffer-save-encrypted
	  (set-buffer-modified-p t)))))

(defun set-des-encryption-ascii (des-ascii &optional buffer)
  "Determine whether ASCII mode is needed for BUFFER.
ASCII should be \"-a\" if ASCII format is resired, or \"\" otherwise.
BUFFER should be a buffer or the name of one;
it defaults to the current buffer.  If BUFFER is in crypt mode, then it is
also marked as modified, since it needs to be saved with the new key."
  (interactive
   (progn
     (barf-if-buffer-read-only)
     (list
      (setq des-ascii (y-or-n-p
       (format "Encrypted data in ASCII format? (%s) "
	       (buffer-name)))))))
  (or buffer (setq buffer (current-buffer)))
  (save-excursion
    (set-buffer buffer)
    (if (equal des-ascii buffer-des-encryption-ascii)
	(message "DES output mode is identical to original, no change.")
      (setq buffer-des-encryption-ascii des-ascii)
      ;; don't touch the modify flag unless we're in crypt-mode.
      (if buffer-save-encrypted
	  (set-buffer-modified-p t)))))

(defun crypt-mode (&optional arg)
  "Toggle crypt mode.
With arg, turn crypt mode on iff arg is positive, otherwise turn it off.
In crypt mode, buffers are automatically encrypted before being written.
If crypt mode is toggled and a key has been set for the current buffer, then
the current buffer is marked modified, since it needs to be rewritten
with (or without) encryption.

Use \\[set-encryption-key] to set the encryption key for the current buffer.

Entering crypt mode causes auto-saving to be turned off in the current buffer,
as there is no way (in Emacs Lisp) to force auto save files to be encrypted."
  (interactive "P")
  (let ((oldval buffer-save-encrypted))
    (setq buffer-save-encrypted
	  (if arg (> arg 0) (not buffer-save-encrypted)))
    (if buffer-save-encrypted
	(auto-save-mode 0)
      (auto-save-mode (if (and auto-save-default buffer-file-name) 1 0)))
    (if buffer-encryption-key
	(set-buffer-modified-p
	 (not (eq oldval buffer-save-encrypted))))))

(defun compact-mode (&optional arg)
  "Toggle compact mode.
With arg, turn compact mode on iff arg is positive, otherwise turn it off.
In compact mode, buffers are automatically compacted before being written.
If compact mode is toggled, the current buffer is marked modified, since
it needs to be written with (or without) compaction.

Entering compact mode causes auto-saving to be turned off in the current
buffer, as there is no way (in Emacs Lisp) to force auto save files to be
compacted."
  (interactive "P")
  (let ((oldval buffer-save-compacted))
    (setq buffer-save-compacted
	  (if arg (> arg 0) (not buffer-save-compacted)))
    (if buffer-save-compacted
	(auto-save-mode 0)
      (auto-save-mode (if (and auto-save-default buffer-file-name) 1 0)))
    (set-buffer-modified-p (not (eq oldval buffer-save-compacted)))))

(defun compress-mode (&optional arg)
  "Toggle compress mode.
With arg, turn compress mode on iff arg is positive, otherwise turn it off.
In compress mode, buffers are automatically compressed before being written.
If compress mode is toggled, the current buffer is marked modified, since
it needs to be written with (or without) compression.

Entering compress mode causes auto-saving to be turned off in the current
buffer, as there is no way (in Emacs Lisp) to force auto save files to be
compressed."
  (interactive "P")
  (let ((oldval buffer-save-compressed))
    (setq buffer-save-compressed
	  (if arg (> arg 0) (not buffer-save-compressed)))
    (if buffer-save-compressed
	(auto-save-mode 0)
      (auto-save-mode (if (and auto-save-default buffer-file-name) 1 0)))
    (set-buffer-modified-p (not (eq oldval buffer-save-compressed)))))




























































(defun read-string-no-echo (prompt &optional confirm)
  "Read a string from the minibuffer, prompting with PROMPT.
Optional second argument CONFIRM non-nil means that the user will be asked
  to type the string a second time for confirmation and if there is a
  mismatch, the process is repeated.

Line editing keys are:
  C-h, DEL	rubout
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
			   (t (throw 'return-value string))))
		    (confirm
		     (setq string (buffer-string))
		     (message (concat prompt "[Retype to confirm...]"))
		     (sit-for 2))
		    (t (throw 'return-value (buffer-string)))))
	  (set-buffer-modified-p nil)
	  (kill-buffer input-buffer))))))




























































;; Install the hooks, then add the mode indicators to
;; the minor mode alist.
(cond
 ((not (memq 'write-crypt-file-hook write-file-hooks))
  ;; make this hook last on purpose
  (setq write-file-hooks (append write-file-hooks
				 (list 'write-crypt-file-hook))
	find-file-hooks (cons 'find-crypt-file-hook find-file-hooks)
	find-file-not-found-hooks (cons 'find-crypt-file-hook
					find-file-not-found-hooks)
	minor-mode-alist (nconc (list '(buffer-save-encrypted " Crypt")
				      '(buffer-save-compacted " Compact")
				      '(buffer-save-compressed " Compress"))
				minor-mode-alist))))


-----------cut here-------------
> It was an obcrypt, because I was writing primarily about non-crypt
> material in sci.crypt.  An ob-foo need not be a throw away remark
> about foo.

Well, not that I quite understood this... Oh, well...
--
Regards,
Uri         uri@watson.ibm.com      scifi!angmar!uri 	N2RIU
-----------
<Disclamer>


