Emacs Manual for myself
=======================

Markdown Mode
-------------
[1.1]: https://github.com/defunkt/markdown-mode
[1.2]: https://guides.github.com/features/mastering-markdown

There are three important links for mastering markdown, emacs markdown mode [markdown-mode.el][1.1], marddown [syntax][1.2] and [github flavored markdown](http://daringfireball.net/projects/markdown/syntax)

- Compile: `C-c C-c m`
- Preview: `C-c C-c p`
- Promotion and Demotion: `C-c C--` and `C-c C-=`
- Editing Lists: `M-RET`, `M-UP`, `M-DOWN`, `M-LEFT`, and `M-RIGHT`
- Horizontal Rules: C-c -
- `C-c C-c c` will check for undefined references

##### Headings: `C-c C-t` #####
+ use `C-c C-t 1` through `C-c C-t 6` for atx (hash mark)
+ and `C-c C-t !` or `C-c C-t @` for setext headings of level one or two

##### Hyperlinks: `C-c C-a` #####
- `C-c C-a l` inserts an inline link of the form `[text](url)`
- `C-c C-a L` inserts a reference link of the form `[text][label]`
- `C-c C-a u` inserts a bare url e.g. <https://github.com>

##### Styles: `C-c C-s` #####
+ `C-c C-s e` italic (emphasis)
+ `C-c C-s s` bold (strong)
+ `C-c C-s c` for inline code (code)

##### Following Links: `C-c C-o` #####
+ Use `M-p` and `M-n` to quickly jump to the previous or next link of any type.


Emacs basic
-----------
[2.1]: http://www.emacswiki.org/emacs/KeyBindingDiscussion
[2.2]: http://www.emacswiki.org/emacs/PrefixKey
[2.3]: http://www.gnu.org/software/emacs/manual/html_node/emacs/Search.html#Search
[2.4]: http://www.gnu.org/software/emacs/manual/html_node/emacs/Moving-Point.html
[2.5]: http://www.gnu.org/software/emacs/manual/html_node/emacs/Query-Replace.html
[2.6]: https://www.gnu.org/software/emacs/manual/html_node/emacs/Registers.html "Registers"

+ `C-x C-o`
  + Delete all but one of many consecutive blank lines (delete-blank-lines).
+ [key binding and prefix key][2.1]

##### [Move][2.4] #####
+ `M-<`
  - Move to the top of the buffer (beginning-of-buffer)
+ `M->`
  - Move to the end of the buffer (end-of-buffer).
+ `M-g M-g (M-g g)`
  - Read a number n and move point to the beginning of line number n (goto-line).
+ `M-g TAB`
  - Read a number n and move to column n in the current line.
+ `M-d`
  - Kill forward to the end of the next word (kill-word).
+ `M-DEL`
  - Kill back to the beginning of the previous word (backward-kill-word).

##### [Search][2.3] #####
+ `C-M-s`
  - Begin incremental regexp search (isearch-forward-regexp).
+ `C-M-r`
  - Begin reverse incremental regexp search (isearch-backward-regexp).

##### [Query Replace][2.5] #####
+ `M-%` string RET newstring RET
  - Replace some occurrences of string With newstring.
+ `C-M-%` regexp RET newstring RET
  - Replace some matches for regexp with newstring

        SPC or y
              to replace the occurrence With newstring.
          DEL or n
              to skip to the next occurrence without replacing this one.
          RET or q
              to exit without doing any more replacements.
          ,
              to replace but not move point immediately,
          !
              to replace all remaining occurrences without asking again.

##### The Mark and the Region #####
The text between point and the mark is called the *region*

+ `C-@ C-SPC`
  - set the mark at point, and activate it (set-mark-command)
+ `C-x C-x`
  - move point where the mark used to be (exchange-point-and-mark)
+ `M-@`
  - Set mark after end of next word (mark-word). This does not move point.
+ `M-h`
  - Move point to the beginning of the current paragraph, and set mark at the end (mark-paragraph)

##### [Registers][2.6] #####
<http://emacswiki.org/emacs/Registers>

+ Position register
  `C-x r SPC r`  point-to-register
  `C-x r j r`    Jump to the position and buffer saved in register r (jump-to-register)
+ Bookmarks
  `C-x r m RET`            Set the bookmark for the visited file, at point.
  `C-x r m bookmark RET`   Set the bookmark named bookmark at point (bookmark-set).
  `C-x r b bookmark RET`   Jump to the bookmark named bookmark (bookmark-jump)
  `C-x r l`                List all bookmarks (list-bookmarks)
  `M-x bookmark-save`      Save all the current bookmark values in the default bookmark file
+ File Names in Registers
  - put a file name into register r:
    `(set-register r '(file . name))`


cscope
------

[cscope.el](https://github.com/dkogan/xcscope.el)
[CScopeAndEmacs](http://emacswiki.org/emacs/CScopeAndEmacs)

``` shell
    apt-get -y install cscope
    cscope -bR
    (add-hook 'c-mode-common-hook '(lambda () (require 'xcscope)))
```

+ operation the cscope buffer.
  n/p navigates over individual results
  k kills individual results
  M-k kills file results
  M-K kills result sets

+ search commands
  `C-c s u` *go back*
  `C-c s G` *find global definition*
  `C-c s c` *called by who*
  `C-c s L` *create list of files to index*

  `M-k` kills file results
  `M-K` kills result sets

  `C-c s s` Find symbol.
  `C-c s =` Find assignments to this symbol
  `C-c s d` Find global definition.
  `C-c s g` Find global definition (alternate binding).
  `C-c s C` Find called functions (list functions called from a function).

  `C-c s b` Display cscope buffer.
  `C-c s B` Auto display cscope buffer toggle.
  `C-c s n` Next symbol.
  `C-c s N` Next file.
  `C-c s p` Previous symbol.
  `C-c s P` Previous file.
