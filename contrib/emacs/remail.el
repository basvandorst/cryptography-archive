;; remail.el, by Jin S. Choi <jsc@mit.edu>
;; Quick hack to allow easy use of cypherpunk remailers.
;; Requires mailcrypt v1.2 or higher.

;; Always encrypts. If you don't want to use encryption, it's simple
;; enough to set up by hand....
;; Assumes that you have the public keys of the remailers you want to
;; use in your keyring already.

(require 'mailcrypt)

(defvar remailer-list
  (list "hfinney@shell.portal.com" "hal@alumni.caltech.edu")
  "*A list of remailers to mail through.")

(defun remail-message ()
  "*Munge the current message to go through the remailers listed in
remailer-list and end up at the address listed in the To: field."
  (interactive)
  (goto-char (point-min))
  (re-search-forward (concat "^" mail-header-separator))
  (beginning-of-line)
  (narrow-to-region (point-min) (point))
  (let ((recipients (mail-fetch-field "to" nil t))
	(remailer-list remailer-list)
	from to)
    (delete-region (point-min) (point-max))
    (goto-char (point-min))
    (insert "To: " (car remailer-list) "\n")
    (setq remailer-list (reverse remailer-list))
    (widen)
    (forward-line)
    (setq to recipients)
    (setq from (car remailer-list))
    (while remailer-list
      (if to 
	  (insert "::\nRequest-Remailing-To: " to "\n\n"))
      (mc-encrypt-message from 'pgp)
      (save-excursion 
	(insert "::\nEncrypted: PGP\n\n"))
      (setq remailer-list (cdr remailer-list))
      (setq to from)
      (setq from (car remailer-list)))))

