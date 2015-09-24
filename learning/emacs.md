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

markdown-enable-math
\[
A^TA = I$
\]


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

+ `M-y`
  - Replace the text just yanked with an [earlier](http://www.gnu.org/software/emacs/manual/html_node/emacs/Earlier-Kills.html#Earlier-Kills) batch of killed text (yank-pop)
+ `M-\ delete-horizontal-space`
+ `C-x C-o`
  + Delete all but one of many consecutive blank lines (delete-blank-lines).
+ [key binding and prefix key][2.1]
+ [A sample .emacs file](https://www.freebsd.org/doc/en/books/developers-handbook/emacs.html), in this link show define key

**Transposing Text**
`C-t` Transpose two characters (transpose-chars).
`M-t` Transpose two words (transpose-words).
`C-M-t` Transpose two balanced expressions (transpose-sexps).
`C-x C-t` Transpose two lines (transpose-lines)

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
<br/> Replace some occurrences of string With newstring.
+ `C-M-%` regexp RET newstring RET
<br/> Replace some matches for regexp with SPC

    * `newstring or y`
        <br/> to replace the occurrence With newstring.
    * `DEL or n`
        <br/> to skip to the next occurrence without replacing this one.
    * `RET or q`
        <br/> to exit without doing any more replacements.
    * `,`
        <br/> to replace but not move point immediately,
    * `!`
        <br/> to replace all remaining occurrences without asking again.



##### The Mark and the Region #####
The text between point and the mark is called the *region*

+ `C-@ C-SPC`
<br/> set the mark at point, and activate it (set-mark-command)
+ `C-x C-x`
<br/> move point where the mark used to be (exchange-point-and-mark)
+ `M-@`
<br/> Set mark after end of next word (mark-word). This does not move point.
+ `M-h`
<br/> Move point to the beginning of the current paragraph, and set mark at the end (mark-paragraph)

##### [Registers][2.6] #####
<http://emacswiki.org/emacs/Registers>

+ Position register
<br/>  `C-x r SPC r`  point-to-register
<br/>  `C-x r j r`    Jump to the position and buffer saved in register r (jump-to-register)
+ Bookmarks
<br/>  `C-x r m RET`            Set the bookmark for the visited file, at point.
<br/>  `C-x r m bookmark RET`   Set the bookmark named bookmark at point (bookmark-set).
<br/>  `C-x r b bookmark RET`   Jump to the bookmark named bookmark (bookmark-jump)
<br/>  `C-x r l`                List all bookmarks (list-bookmarks)
<br/>  `M-x bookmark-save`      Save all the current bookmark values in the default bookmark file
+ File Names in Registers
<br/> put a file name into register r: `(set-register r '(file . name))`


TeX LaTeX AUCTeX
----------------

**Font** <br/>
`C-c C-f C-b`
    Insert bold face '\textbf{}' text.
<br/>
`C-c C-f C-i`
    Insert italics '\textit{}' text.
<br/>
`C-c C-f C-e`
    Insert emphasized '\emph{}' text.
<br/>
`C-c C-f C-s`
    Insert slanted '\textsl{}' text.
<br/>
`C-c C-f C-r`
    Insert roman \textrm{} text.
<br/>
`C-c C-f C-f`
    Insert sans serif '\textsf{}' text.
<br/>
`C-c C-f C-t`
    Insert typewriter '\texttt{}' text.
<br/>
`C-c C-f C-c`
    Insert SMALL CAPS '\textsc{}' text.
<br/>
`C-c C-f C-d`
    Delete the innermost font specification containing point.

**Entering Mathmatics** <br/>
`C-c ~` Toggle LaTeX Math mode.

**Marking** <br/>
`C-c .` mark the current environment
`C-c *` mark the current section

**Folding Macros and Environments** <br/>
`TeX-fold-mode` activate the mode in a certain buffer by `C-c C-o C-f`
`TeX-fold-buffer C-c C-o C-b` Hide all foldable items in the current buffer
`TeX-fold-region C-c C-o C-r` Hide all configured macros in the marked region.
`TeX-fold-macro C-c C-o C-m` Hide the macro on which point currently is located.
`TeX-fold-envC-c C-o C-e` Hide the environment on which point currently is located
`TeX-fold-comment C-c C-o C-c` Hide the comment point is located on
`TeX-fold-dwim C-c C-o C-o`  Hide or show items according to the current context. If there is folded content, unfold it. If there is a marked region, fold all configured content in this region. If there is no folded content but a macro or environment, fold it.

**Help**
`C-c ?` Get documentation about macros, packages or TeX & Co

cscope
------

[cscope.el](https://github.com/dkogan/xcscope.el)
[CScopeAndEmacs](http://emacswiki.org/emacs/CScopeAndEmacs)

    apt-get -y install cscope
    cscope -bR
    (add-hook 'c-mode-common-hook '(lambda () (require 'xcscope)))


+ operation the cscope buffer.
<br/>  `n/p` navigates over individual results
<br/>  `k` kills individual results
<br/>  `M-k` kills file results
<br/>  `M-K` kills result sets

+ search commands
<br/>  `C-c s u` *go back*
<br/>  `C-c s G` *find global definition*
<br/>  `C-c s c` *called by who*
<br/>  `C-c s L` *create list of files to index*
<br/>
<br/>  `M-k` kills file results
<br/>  `M-K` kills result sets
<br/>
<br/>  `C-c s s` Find symbol.
<br/>  `C-c s =` Find assignments to this symbol
<br/>  `C-c s d` Find global definition.
<br/>  `C-c s g` Find global definition (alternate binding).
<br/>  `C-c s C` Find called functions (list functions called from a function).
<br/>
<br/>  `C-c s b` Display cscope buffer.
<br/>  `C-c s B` Auto display cscope buffer toggle.
<br/>  `C-c s n` Next symbol.
<br/>  `C-c s N` Next file.
<br/>  `C-c s p` Previous symbol.
<br/>  `C-c s P` Previous file.
