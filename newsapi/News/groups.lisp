
(defvar *small-list*
  '(("news" "software" "readers")
    ("news" "software" "notes")
    ("news" "software" "nntp")
    ("news" "software" "nn")
    ("news" "software" "b")
    ("news" "software" "anu-news")
    ("news" "newusers" "questions")
    ("news" "newsites")
    ("news" "misc")
    ("news" "lists" "ps-maps")
    ("news" "lists")
    ("news" "groups")
    ("news" "future")
    ("news" "config")
    ("news" "answers")
    ("news" "announce" "newusers")
    ("news" "announce" "newgroups")
    ("news" "announce" "important")
    ("news" "announce" "conferences")
    ("news" "admin" "technical")
    ("news" "admin" "policy")
    ("news" "admin" "misc")
    ("news" "admin")))

(defvar *current-list* nil)
(defvar *current-group* nil)
(defvar *rising* 0)


;;; returns the number of groupname elements that match 
;;; i.e., (compare-name-depth "news.software.nntp" "news.software.nn")
;;; will return 2

(defun compare-name-depth (a b)
  (cond ((or (null a) (null b)) 0)
	((string-equal (car a) (car b))
	 (1+ (compare-name-depth (cdr a) (cdr b))))
	(t 0)))

;;; The idea is that since we're constructing the tree from a
;;; reverse-sorted list, when we come upon a group outside of the current
;;; subtree, it will be the deepest leaf in a new subtree.  For example,
;;; in a reverse sorted list, we will run run into alt.barney.die.die.die
;;; _before_ we run into a.b.die.die or a.b.die, or any other.  So, by
;;; 'diving' (recursing), we create a whole branch of the tree at once.
;;; Using our example, we can create:
;;; 
;;; alt
;;;    barney
;;;          die
;;;             die
;;;                die
;;; 
;;; Let's say the next group (in the reverse-sorted list) is
;;; 'alt.barney.stinks'.  group-name-depth tells us that the new group
;;; shares 2 group-name elements with the current group.  taking the
;;; difference tells us that we need to 'rise' up the tree 3 levels.
;;; 
;;; alt
;;;    barney <--------+ 
;;;          die       ^
;;;             die    ^
;;;                die ^
;;; 
;;; This is done by setting the *rising* global variable, which is 
;;; decremented at each return.
;;; 
;;; At this point (2 deep into the recursion), we have collected (via the
;;; return values while 'rising') a sub-tree, its value is
;;; 
;;; (die (die (die)))
;;; 
;;; We now continue on (recursing) for each other child in the alt.barney
;;; tree.  If any of these recursive calls has set the *rising* variable,
;;; then we know there are no more children at this level, and return
;;; a list of all the child-trees we collected.  At this point in the
;;; example (after collecting whatever's in the alt.barney.stinks tree)
;;; we have
;;; 
;;; ( (die (die (die))) (stinks) )
;;; 
;;; Let's say the next group is completely different -
;;; news.software.barney.  That group will set the *rising* variable to 3
;;; (all the way up to the root of the tree), and our collection will now
;;; look like this:
;;; 
;;; (root
;;;  (news
;;;   (software
;;;    (barney)))
;;;  (alt
;;;   (barney
;;;    (die
;;;     (die
;;;      (die)))
;;;    (stinks)))

(defun construct-group-tree (depth)

  ;; if we're at a leaf node

  (if (= depth (length *current-group*))

      ;; grab the next group, and figure
      ;; out how far up the tree to rise

      (progn
	(setq *rising*
	      (- depth
		 (compare-name-depth
		  *current-group*
		  (car *current-list*))
		 ))
	(setq *current-group* (pop *current-list*))
        ;; this is a place-holder for a group object
	'group
	)
    ;; else
    (progn
      ;; for each group at this level
      (do ((result nil)
	   (name (elt *current-group* depth)))

	  ((/= *rising* 0)
	   (progn
             ;; going up!
	     (decf *rising*)
	     (cons name result)))

        ;; collect subtrees until
        ;; *rising* is set, meaning we are done with
        ;; this subtree.

	(push 
	 (construct-group-tree
	  (1+ depth))
	 result)))))

(defun build-it ()
  ;; assumption that it's in reverse alpha order
  (setq *current-list* *small-list*)
  (setq *current-group* (pop *current-list*))
  (setq *rising* 0)
  (construct-group-tree 0))

(defun print-group-tree (tree depth)
  (cond ((null tree) nil)
	((listp tree)
	 (dolist (x tree)
	   (print-group-tree x (1+ depth))))
	((equal tree 'group) (princ "."))
	(t
	 (terpri)
	 (dotimes (x depth)
	   (princ " "))
	 (princ tree)
	 )))

(defun test ()
  (progn 
    (print-group-tree (build-it) 0)
    (terpri)))

(test)

;;; news
;;;  admin.
;;;   misc.
;;;   policy.
;;;   technical.
;;;  announce
;;;   conferences.
;;;   important.
;;;   newgroups.
;;;   newusers.
;;;  answers.
;;;  config.
;;;  future.
;;;  groups.
;;;  lists.
;;;   ps-maps.
;;;  misc.
;;;  newsites.
;;;  newusers
;;;   questions.
;;;  software
;;;   anu-news.
;;;   b.
;;;   nn.
;;;   nntp.
;;;   notes.
;;;   readers.
