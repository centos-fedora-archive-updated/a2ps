;;; a2ps.el --- Major mode for a2ps style sheets

;; Author: Akim Demaille (demaille@inf.enst.fr)
;; Maintainer: Akim Demaille (demaille@inf.enst.fr)
;; Keywords: languages, faces, a2ps

;;; Copyright (c) 1988, 89, 90, 91, 92, 93 Miguel Santana
;;; Copyright (c) 1995, 96, 97, 98 Akim Demaille, Miguel Santana

;;;
;;; This file is part of a2ps.
;;;
;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3, or (at your option)
;;; any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program; see the file COPYING.  If not, write to
;;; the Free Software Foundation, 59 Temple Place - Suite 330,
;;; Boston, MA 02111-1307, USA.
;;;

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.ssh\\'" . a2ps-mode))

;; Name or full path of program invoked for a2ps
(defvar a2ps-program "a2ps")

;;thank god for make-regexp.el!
(defvar a2ps-font-lock-keywords
  `(
    ("^\\\#.*" . font-lock-comment-face)
;    ("\\\$\\\*" . font-lock-variable-name-face)
;    ("\\\$[0-9]" . font-lock-variable-name-face)
;    ("\\\$\\\#" . font-lock-variable-name-face)
    ;; Keywords related to the LaTeX symbols
    ("\\b\\(---\\|\\\\\\(Alpha\\|Beta\\|Chi\\|D\\(elta\\|ownarrow\\)\\|E\\(psilon\\|ta\\)\\|Gamma\\|I\\(m\\|ota\\)\\|Kappa\\|L\\(ambda\\|eft\\(arrow\\|rightarrow\\)\\)\\|Mu\\|Nu\\|Om\\(ega\\|icron\\)\\|P\\(hi\\|i\\|si\\)\\|R\\(e\\|ho\\|ightarrow\\)\\|Sigma\\|T\\(au\\|heta\\)\\|Up\\(arrow\\|silon\\)\\|Xi\\|Zeta\\|a\\(l\\(eph\\|pha\\)\\|ngle\\|pp\\(le\\|rox\\)\\)\\|b\\(eta\\|ullet\\)\\|c\\(a\\(p\\|rriagereturn\\)\\|dot\\|hi\\|irc\\|lubsuit\\|o\\(ng\\|pyright\\)\\|up\\)\\|d\\(elta\\|i\\(amondsuit\\|v\\)\\|ownarrow\\)\\|e\\(mptyset\\|psilon\\|quiv\\|ta\\|xists\\)\\|f\\(lorin\\|orall\\)\\|g\\(amma\\|eq\\)\\|heartsuit\\|i\\(n\\(\\|fty\\|t\\)\\|ota\\)\\|kappa\\|l\\(a\\(mbda\\|ngle\\)\\|ceil\\|dots\\|e\\(ft\\(arrow\\|rightarrow\\)\\|q\\)\\|floor\\)\\|mu\\|n\\(abla\\|eq\\|ot\\(\\|\\\\\\(in\\|subset\\)\\)\\|u\\)\\|o\\(m\\(ega\\|icron\\)\\|plus\\|times\\)\\|p\\([im]\\|artial\\|erp\\|hi\\|r\\(ime\\|o\\(d\\|pto\\)\\)\\|si\\)\\|r\\(a\\(dicalex\\|ngle\\)\\|ceil\\|egister\\|floor\\|ho\\|ightarrow\\)\\|s\\(i\\(gma\\|m\\)\\|padesuit\\|u\\(bset\\(\\|eq\\)\\|chthat\\|m\\|pset\\(\\|eq\\)\\|rd\\)\\)\\|t\\(au\\|he\\(refore\\|ta\\)\\|imes\\|rademark\\)\\|up\\(arrow\\|silon\\)\\|v\\(ar\\(Upsilon\\|copyright\\|diamondsuit\\|p\\(hi\\|i\\)\\|register\\|sigma\\|t\\(heta\\|rademark\\)\\)\\|ee\\)\\|w\\(edge\\|p\\)\\|xi\\|zeta\\)\\)\\b" . font-lock-type-face)
    ;; Keywords related to the struture
    ("\\b\\(a\\(2ps\\|lphabets?\\|ncestors\\|re\\)\\|by\\|c\\(ase\\|losers\\)\\|documentation\\|e\\(nd\\|xceptions\\)\\|first\\|i\\([ns]\\|nsensitive\\)\\|keywords\\|op\\(erators\\|tional\\)\\|requires\\|s\\(e\\(cond\\|nsitive\\|quences\\)\\|tyle\\)\\|version\\|written\\)\\b" . font-lock-keyword-face)
    ;; Keywords related to the faces or special sequences
    ("\\b\\(C\\(-\\(char\\|string\\)\\|omment\\(\\|_strong\\)\\)\\|E\\(ncoding\\|rror\\)\\|In\\(dex[1234]\\|visible\\)\\|Keyword\\(\\|_strong\\)\\|Label\\(\\|_strong\\)\\|Plain\\|S\\(tring\\|ymbol\\)\\|Tag[1234]\\)\\b" . font-lock-type-face)
    "default font-lock-keywords")
)

;; this may still need some work
(defvar a2ps-mode-syntax-table nil
  "syntax table used in a2ps mode")
(setq a2ps-mode-syntax-table (make-syntax-table))
;(modify-syntax-entry ?` "('" a2ps-mode-syntax-table)
;(modify-syntax-entry ?' ")`" a2ps-mode-syntax-table)
(modify-syntax-entry ?# "<\n" a2ps-mode-syntax-table)
(modify-syntax-entry ?\n ">#" a2ps-mode-syntax-table)
(modify-syntax-entry ?/  "\"" a2ps-mode-syntax-table)
;(modify-syntax-entry ?{  "_" a2ps-mode-syntax-table)
;(modify-syntax-entry ?}  "_" a2ps-mode-syntax-table)
(modify-syntax-entry ?\\  "_" a2ps-mode-syntax-table)
(modify-syntax-entry ?@  "w" a2ps-mode-syntax-table)
(modify-syntax-entry ?_  "w" a2ps-mode-syntax-table)
(modify-syntax-entry ?*  "w" a2ps-mode-syntax-table)

(defvar a2ps-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-c\C-b" 'a2ps-a2ps-buffer)
;;;    (define-key map "\C-c\C-r" 'a2ps-a2ps-region)
    (define-key map "\C-c\C-c" 'comment-region)
    map))

(defun a2ps-check-buffer ()
  "Check that the current style sheet is correct"
  (interactive)
  ;; This `let' is for protecting the previous value of compile-command.
  (let ((compile-command (concat a2ps-program
				 " -q < /dev/null -o - > /dev/null -E"
				 buffer-file-name)))
    (compile compile-command))
)

(defun a2ps-a2ps-region ()
  "send contents of the current region to a2ps"
  (interactive)
  (start-process "a2psprocess" "*a2ps output*" a2ps-program "-e")
  (process-send-region "a2psprocess" (point) (mark))
  (process-send-eof "a2psprocess")
  (switch-to-buffer "*a2ps output*")
)

(easy-menu-define a2ps-mode-menu
    a2ps-mode-map
    "Menu used in a2ps mode."
  (list "a2ps"
        ["Documentation" a2ps-goto-info-page t]
	))

;; Open info on the page on How to write a style sheet
(defun a2ps-goto-info-page ()
  "Read documentation for a2ps style sheets in the info system."
  (interactive)
  (require 'info)
  (Info-goto-node "(a2ps)Style sheets implementation"))

;;;###autoload
(defun a2ps-mode ()
  "A major-mode to edit a2ps style sheet files
\\{a2ps-mode-map}
"
  (interactive)
  (kill-all-local-variables)
  (use-local-map a2ps-mode-map)

  (make-local-variable 'comment-start)
  (setq comment-start "#")
  (make-local-variable 'parse-sexp-ignore-comments)
  (setq parse-sexp-ignore-comments t)

  ; Used for the menus
  (require 'easymenu)

  ; Install the menus
  (easy-menu-add a2ps-mode-menu a2ps-mode-map)

  ; The \ is used both as an escape in strings, and
  ; as a symbol constituent in LaTeX like symbols
  (setq words-include-escapes t)

  (make-local-variable	'font-lock-defaults)
  (setq major-mode 'a2ps-mode
	mode-name "a2ps"
	font-lock-defaults `(a2ps-font-lock-keywords nil)
	)
  (set-syntax-table a2ps-mode-syntax-table)
  (run-hooks 'a2ps-mode-hook))

(provide 'a2ps-mode)

;;; a2ps.el ends here
