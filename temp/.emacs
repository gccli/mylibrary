;;
;; Basic settings
;;
(define-key global-map "\C-r" 'goto-line)
(define-key global-map "\C-h" 'replace-string)
(global-hl-line-mode 1)

;; deal with white spaces
(require 'whitespace)
(global-whitespace-mode)
(setq whitespace-style
      '(face trailing tabs lines lines-tail empty
     space-after-tab space-before-tab))
(add-hook 'before-save-hook 'delete-trailing-whitespace)

;; set backup
(setq
 backup-by-copying t           ; don't clobber symlinks
 backup-directory-alist
 '((""."~/.emacs.d/backups"))  ; don't litter my fs tree
 delete-old-versions t
 kept-new-versions 3
 kept-old-versionps 1
 version-control t)            ; use versioned backups

;; cc-mode
(setq c-default-style "linux" c-basic-offset 4)
(setq-default indent-tabs-mode nil)

; For org-mode
(require 'org-install)
(require 'org-publish)
(add-to-list 'auto-mode-alist '("\\.org\\'" . org-mode))
(add-to-list 'auto-mode-alist '("\\.txt\\'" . org-mode))
(add-hook 'org-mode-hook 'turn-on-font-lock)
(add-hook 'org-mode-hook
(lambda () (setq truncate-lines nil)))
(global-set-key "\C-cl" 'org-store-link)
(global-set-key "\C-ca" 'org-agenda)
(global-set-key "\C-cb" 'org-iswitchb)

; PHP-mode
(load-file "~/.emacs.d/php-mode.el")
(autoload 'php-mode "php-mode" "Major mode for editing php code." t)
(add-to-list 'auto-mode-alist '("\\.php$" . php-mode))
(add-to-list 'auto-mode-alist '("\\.inc$" . php-mode))

; cscope
; (load-file "/usr/share/emacs/site-lisp/xcscope.el")
(add-hook 'c-mode-common-hook '(lambda () (require 'xcscope)))
