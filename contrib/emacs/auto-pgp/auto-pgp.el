;; Automatic PGP encryption/decryption for emacs.
;; Copyright (C) 1993 Ian Jackson.   This file is part of Auto-PGP.
;;
;; This file is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; It is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


;; Don't touch the lines with `provide' and `defvar' or the formfeeds
;; around this section - the configuration program will mess
;; about with them. You can edit the documentation strings though.

(provide 'auto-pgp)

(if (not (boundp 'expand-mail-aliases))
    (load-library "mailalias")) ; there's no provide in mailalias.el !

(defvar autopgp-normal-program "pgp"
  "Name to use to run PGP when no passphrase required")

(defvar autopgp-sign-program "pgp-auto"
  "Name to use to run PGP when a passphrase is required")

(defvar autopgp-decrypt-filter-program "pgp-decrypt"
  "Automatic decryption filter")

;; End of configuration block.


(defvar autopgp-outgoing-address-translation-alist ()
  "Alist of name->keyname mappings. Before key lookup but after
mail alias translation and `user@site (Name)' to `Name <user@site>'
conversion each recipient will be looked up in this list using assoc
to try to find a replacement for the purposes of finding the user's
key.

The translated recipients are only used for keyring search, and
are not reinserted in the header of the message being encrypted.")

(define-key ctl-x-map "yy" 'autopgp-encrypt-and-sign-message-in-buffer)
(define-key ctl-x-map "ye" 'autopgp-encrypt-message-in-buffer)
(define-key ctl-x-map "ys" 'autopgp-sign-message-in-buffer)
(define-key ctl-x-map "ya" 'autopgp-sign-buffer)
(define-key ctl-x-map "yd" 'autopgp-decrypt-message-or-check-sig)
(define-key ctl-x-map "y?" 'autopgp-help)

(defun autopgp-help ()
  "Show list of Emacs Auto-PGP functions on C-x y"
  (interactive)
  (with-output-to-temp-buffer "*Help*"
    (princ
     (let ((autopgpmap (lookup-key ctl-x-map "y")))
       (substitute-command-keys
        "The following Auto-PGP commands are available on C-x y:
\\{autopgpmap}
Use \\[describe-key] (describe-key) or \\[describe-function] (describe-function)\
 for more information.")))
    (print-help-return-message)))

(defun autopgp-encrypt-and-sign-message-in-buffer ()
  "Sign and encrypt the message body, scaning the header
of the message in the current buffer for the recipients."
  (interactive)
  (autopgp-outgoing (autopgp-message-start) (autopgp-message-end)
                    autopgp-sign-program "se" (autopgp-scan-recipients)))

(defun autopgp-sign-message-in-buffer ()
  "Sign the body of the message in the current buffer."
  (interactive)
  (autopgp-outgoing (autopgp-message-start) (autopgp-message-end)
                    autopgp-sign-program "s" nil))

(defun autopgp-sign-buffer ()
  "Sign the whole of (the visible portion of) the current buffer."
  (interactive)
  (autopgp-outgoing (point-min) (point-max) autopgp-sign-program "s" nil))

(defun autopgp-encrypt-message-in-buffer ()
  "Encrypt the message body, scanning the header the recipients."
  (interactive)
  (autopgp-outgoing (autopgp-message-start) (autopgp-message-end)
                    autopgp-normal-program "e" (autopgp-scan-recipients)))

(defun autopgp-outgoing (source-start source-end program action recipients)
  "Runs PROGRAM (which must work like PGP) on the text between
SOURCE-START and SOURCE-END, asking the user to confirm whether to
replace it with the processed version, which will have been
put in a .asc file by PROGRAM.

PROGRAM should be either autopgp-normal-program - if no passphrase
is required - or autopgp-sign-program - if a passphrase will be required.

ACTION is a string of action letters to pass to PGP. It will be
preceded by a `-' and followed by `at'. PGP will also be passed the
+clearsig, +batchmode and +force options.

RECIPLIST is the list of recipients to supply to PGP."
  (let ((source-buf (current-buffer))
        (messages-buf (get-buffer-create "*autopgp messages*"))
        replacement-start replacement-end offer-replace)
    (save-excursion
      (set-buffer messages-buf) (setq buffer-read-only nil) (erase-buffer)
      (set-buffer source-buf)
      (setq offer-replace
            (and
             (let ((rc 
                    (apply 'call-process-region source-start source-end
                           program nil messages-buf nil
                           "+batchmode" "+clearsig" "+encrypttoself" "+force"
                           (concat "-" action "aft")
                           recipients)))
               (or (null rc) (zerop rc)))
             (set-buffer messages-buf)
             (goto-char (point-min))
             (search-forward "-----BEGIN PGP " nil t)
             (setq replacement-start (match-beginning 0))
             (goto-char (point-max))
             (zerop (forward-line -1))
             (looking-at "\\-\\-\\-\\-\\-END PGP .*\\-\\-\\-\\-\\-\n")
             (setq replacement-end (point-max)))))
    (autopgp-display-output-perhaps-replace
     offer-replace source-start source-end
     messages-buf nil replacement-start replacement-end)))

(defun autopgp-message-start ()
  (save-excursion
    (goto-char (point-min))
    (re-search-forward "^-\\|^[ \t]*$")
    (if (zerop (forward-line)) () (error "there is no message body"))
    (point)))

(defun autopgp-message-end ()
  (save-excursion
    (goto-char (point-max))
    (re-search-backward "[^ \n\t]")
    (if (zerop (forward-line)) () (error "buffer doesn't end in newline"))
    (point)))

(defun autopgp-scan-recipients ()
  (save-excursion
    (goto-char (point-min))
    (re-search-forward "^-\\|^[ \t]*$" nil t)
    (beginning-of-line)
    (let ((hmin (point-min))
          (hmax (point))
          (orgbuf (current-buffer))
          (tbuf (generate-new-buffer " *autopgp recipients list*"))
          (rlist '()))
      (unwind-protect
          (progn
            (set-buffer tbuf)
            (insert-buffer-substring orgbuf hmin hmax)
            (expand-mail-aliases (point-min) (point-max))
            (goto-char (point-min))
            (while (re-search-forward "[ \t]*\n[ \t]+" nil t)
              (delete-region (match-beginning 0) (point))
              (insert " "))
            (goto-char (point-min))
            (while (let ((case-fold-search t))
                     (re-search-forward "^\\(To\\|C[Cc]\\|[Bb][Cc][Cc]\\):" nil t))
              (let ((fmin (point))
                    (fmax (progn (end-of-line) (insert ",") (point)))
                    amin)
                (goto-char fmin)
                (while
                    (and
                     (re-search-forward "[^\t ]" fmax t)
                     (progn
                       (backward-char 1)
                       (setq amin (point)))
                     (search-forward "," fmax t))
                  (backward-char 1)
                  (if (/= amin (point))
                      (let ((cr (buffer-substring amin (point))))
                        (if (string-match "^\\([^<>()]+\\) (\\(.*\\))$" cr)
                            (setq cr (concat
                                      (substring cr (match-beginning 2) (match-end 2))
                                      " <"
                                      (substring cr (match-beginning 1) (match-end 1))
                                      ">")))
                        (while
                            (let ((el (assoc
                                       cr
                                       autopgp-outgoing-address-translation-alist)))
                              (and el (setq cr (cdr el)))))
                        (setq rlist (cons cr rlist))))
                  (forward-char 1)))))
        (kill-buffer tbuf))
      (if (not rlist) (error "No recipients found for message"))
      (reverse rlist))))

(defun autopgp-decrypt-message-or-check-sig (block-number)
  "Decrypt or check the signature of the current message/buffer.

If there is more than one PGP block it acts on the
BLOCK-NUMBER'th one; otherwise BLOCK-NUMBER should be `1'.

The processing is done by running the PGP decryption wrapper as
a subprocess. If a passphrase is required it must be supplied to
that wrapper via its usual socket mechanism."
  (interactive "p")
  (let ((messages-buf (get-buffer-create "*autopgp messages*"))
        (source-buf (current-buffer))
        source-start source-contents-end offer-replace prefix
        ending-line-start source-end
        replacement-start replacement-end)
    (save-excursion
      (goto-char (point-min))
      (re-search-forward "^\\(\\(- \\)*\\)-----BEGIN PGP.* MESSAGE" nil nil block-number)
      (setq source-start (match-end 1)
            prefix (buffer-substring (match-beginning 0) source-start))
      (search-forward (concat "\n" prefix "-----END PGP"))
      (setq source-contents-end (+ (match-beginning 0) 1)
            ending-line-start (- (match-end 0) 12)
            source-end (progn (end-of-line nil) (point)))
      (set-buffer messages-buf) (setq buffer-read-only nil) (erase-buffer)
      (insert-buffer-substring source-buf source-start source-contents-end)
      (insert-buffer-substring source-buf ending-line-start source-end)
      (insert "\n")
      (goto-char (point-min))
      (if (zerop (length prefix)) ()
        (while (search-forward (concat "\n" prefix) nil t)
          (beginning-of-line) (delete-char 2)))
      (setq offer-replace
            (and
             (let ((rc 
                    (call-process-region (point-min) (point-max)
                                         autopgp-decrypt-filter-program
                                         t t nil)))
               (or (null rc) (zerop rc)))
             (goto-char (point-min))
             (re-search-forward
              "\nEnclosed message text (\\([0-9]+\\) bytes):\n" nil t)
             (= (+ (string-to-int
                    (buffer-substring (match-beginning 1) (match-end 1)))
                   (point))
                (point-max))
             (let ((orgpoint (point)))
               (prog1 (search-backward "Enclosed")
                 (delete-region (point) orgpoint))))
            replacement-start (point)
            replacement-end (progn
                              (goto-char (point-max))
                              (skip-chars-backward "\n" 1)
                              (point))))
    (autopgp-display-output-perhaps-replace
     offer-replace source-start source-end
     messages-buf t replacement-start replacement-end)))

(defun autopgp-display-output-perhaps-replace
  (offer-replace source-start source-end
   messages-buf selective replacement-start replacement-end)
  "Displays results from PGP invocation and perhaps allows user
to replace some of the current buffer with PGP output.

If OFFER-REPLACE is non-nil the user will be offered the choice to
replace current buffer from SOURCE-START to SOURCE-END with text from
MESSAGES-BUF (see below).

If SELECTIVE is non-nil MESSAGES-BUF will be initially displayed with
all lines starting with a space hidden and the user will be given the
option of switching to full display; otherwise no hiding will be done.

If OFFER-REPLACE is set, the part of MESSAGES-BUF from
REPLACEMENT-START to REPLACEMENT-END is removed from MESSAGES-BUF,
and placed in the *autopgp output* buffer, and it is also displayed to
allow the user to decide whether to substitute it into SOURCE-BUF."

  (let ((source-buf (current-buffer))
        output-buf)
    (make-local-variable 'autopgp-original-local-map)
    (make-local-variable 'autopgp-original-mode-name)
    (make-local-variable 'autopgp-original-major-mode)
    (make-local-variable 'autopgp-original-read-only)
    (make-local-variable 'autopgp-source-start)
    (make-local-variable 'autopgp-source-end)

    (setq autopgp-original-local-map (current-local-map)
          autopgp-original-mode-name mode-name
          autopgp-original-major-mode major-mode
          autopgp-original-read-only buffer-read-only
          autopgp-source-start (copy-marker source-start)
          autopgp-source-end (copy-marker source-end))

    (make-local-variable 'autopgp-original-window-config)
    (make-local-variable 'autopgp-output-buffer)
    (make-local-variable 'autopgp-messages-buffer)

    (setq autopgp-original-window-config (current-window-configuration)
          autopgp-output-buffer output-buf
          autopgp-messages-buffer messages-buf)
    (if selective (progn (set-buffer messages-buf)
                         (setq selective-display 1
                               selective-display-ellipses nil)))
    (if offer-replace
        (progn
          (setq output-buf (get-buffer-create "*autopgp output*"))
          (set-buffer output-buf) (setq buffer-read-only nil) (erase-buffer)
          (insert-buffer-substring messages-buf replacement-start replacement-end)
          (switch-to-buffer messages-buf) (setq buffer-read-only nil)
          (delete-region replacement-start replacement-end)
          (goto-char (point-min))
          (delete-other-windows)
          (switch-to-buffer-other-window output-buf)
          (goto-char (point-min)))
      (switch-to-buffer source-buf)
      (delete-other-windows)
      (goto-char (point-min))
      (switch-to-buffer-other-window messages-buf)
      (goto-char (point-min)))

    (if output-buf (progn (set-buffer output-buf)
                          (autopgpquery-mode source-buf)))
    (set-buffer source-buf)
    (setq autopgp-output-buffer output-buf)
    (autopgpquery-mode source-buf)
    (set-buffer messages-buf)
    (autopgpquery-mode source-buf))
  (message
   (if offer-replace
       "space=Y)es=R)eplace  N)no=Q)uit  L)eave-displayed  V)erbose  udUD=scroll  ?=help"
     "space=Q)uit  L)eave-displayed  V)erbose  u/d/U/D=scroll  ?=help")))

(defvar autopgpquery-mode-map ()
  "Keymap used in Auto-PGP Query mode.")

(if autopgpquery-mode-map
    ()
  (setq autopgpquery-mode-map (make-sparse-keymap))
  (define-key autopgpquery-mode-map "v" 'autopgpquery-verbose-toggle)
  (define-key autopgpquery-mode-map "t" 'autopgpquery-verbose-toggle)
  (define-key autopgpquery-mode-map "n" 'autopgpquery-quit)
  (define-key autopgpquery-mode-map "q" 'autopgpquery-quit)
  (define-key autopgpquery-mode-map "y" 'autopgpquery-replace-with-output)
  (define-key autopgpquery-mode-map "r" 'autopgpquery-replace-with-output)
  (define-key autopgpquery-mode-map "a" 'autopgpquery-leave-displayed)
  (define-key autopgpquery-mode-map "l" 'autopgpquery-leave-displayed)
  (define-key autopgpquery-mode-map "?" 'describe-mode)
  (define-key autopgpquery-mode-map " " 'autopgpquery-default-action)
  (define-key autopgpquery-mode-map "D" 'scroll-other-window)
  (define-key autopgpquery-mode-map "U" 'scroll-down-other-window)
  (define-key autopgpquery-mode-map "d" 'scroll-up)
  (define-key autopgpquery-mode-map "u" 'scroll-down))

(defun scroll-down-other-window (arg)
  "Scroll text of next window down ARG lines; or near full screen if no ARG.
The next window is the one below the current one; or the one at the top
if the current one is at the bottom.
When calling from a program, supply a number as argument or nil.

Defined in auto-pgp.el as part of Emacs Auto-PGP."
  (interactive "P")
  (if arg (setq arg (prefix-numeric-value arg)))
  (other-window 1)
  (scroll-down arg)
  (other-window -1))  

(defun autopgpquery-mode (original-buffer)
  "Major mode for accepting/rejecting/viewing an Auto-PGP transaction.
\\{autopgpquery-mode-map}"
  (make-local-variable 'autopgpquery-original-buffer)
  (use-local-map autopgpquery-mode-map)
  (setq autopgpquery-original-buffer original-buffer
        buffer-read-only t
        major-mode 'autopgpquery-mode
        mode-name "AutoPGP-Query"))

(defun autopgpquery-verbose-toggle ()
  "Toggles verbose display of Auto-PGP messages"
  (interactive)
  (save-excursion
    (if (eq major-mode 'autopgpquery-mode)
        (set-buffer autopgpquery-original-buffer))
    (set-buffer autopgp-messages-buffer)
    (setq selective-display (if selective-display nil 1)))
  (redraw-display))

(defun autopgpquery-restore-windows ()
  (condition-case nil
      (progn (save-excursion (bury-buffer autopgp-messages-buffer))
             (save-excursion (bury-buffer autopgp-output-buffer)))
    (error nil))
  (set-window-configuration autopgp-original-window-config))

(defun autopgpquery-assert-mode ()
  (if (not (and (eq major-mode 'autopgpquery-mode)
                (bufferp autopgpquery-original-buffer)))
      (error "This buffer isn't part of an outstanding Auto-PGP transaction."))
  (pop-to-buffer autopgpquery-original-buffer)
  (if (not (eq major-mode 'autopgpquery-mode))
      (error "The Auto-PGP transaction has been cancelled already.")))

(defun autopgpquery-restore-mode ()
  (if (markerp autopgp-source-start)
      (set-marker autopgp-source-start nil))
  (if (markerp autopgp-source-end)
      (set-marker autopgp-source-end nil))
  (setq mode-name autopgp-original-mode-name
        major-mode autopgp-original-major-mode
        buffer-read-only autopgp-original-read-only)
  (use-local-map autopgp-original-local-map)
  (set-buffer-modified-p (buffer-modified-p)))

(defun autopgpquery-default-action ()
  "Default action for Auto-PGP transaction; is replace if there was no
error and output was produced, quit (restoring windows) otherwise."
  (interactive)
  (autopgpquery-assert-mode)
  (if autopgp-output-buffer (autopgpquery-replace-with-output) (autopgpquery-quit)))

(defun autopgpquery-replace-with-output ()
  "Accepts the offer to perform the replacement of the input message
with the Auto-PGP processed version."
  (interactive)
  (autopgpquery-assert-mode)
  (if (not autopgp-output-buffer)
      (error "`Yes' or `Replace' not possible - nothing to replace with"))
  (let ((buffer-read-only nil))
    (kill-region autopgp-source-start autopgp-source-end)
    (goto-char autopgp-source-start)
    (insert (save-excursion (set-buffer autopgp-output-buffer)
                            (buffer-string)))
    (goto-char autopgp-source-start))
  (autopgpquery-restore-windows)
  (autopgpquery-restore-mode))

(defun autopgpquery-quit ()
  "Declines the offer to perform the replacement of the input message
with the Auto-PGP processed version.

The window configuration is restored to that before Auto-PGP was invoked."
  (interactive)
  (autopgpquery-assert-mode)
  (autopgpquery-restore-windows)
  (autopgpquery-restore-mode))

(defun autopgpquery-leave-displayed ()
  "Declines the offer to perform the replacement of the input message
with the Auto-PGP processed version.

The window configuration is left as is, with the PGP messages and perhaps
output visible."
  (interactive)
  (autopgpquery-assert-mode)
  (autopgpquery-restore-mode))
