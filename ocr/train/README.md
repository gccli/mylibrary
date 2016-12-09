Training Tesseract-OCR
======================

Generate Training Images and Box Files
--------------------------------------

1. prepare text training samples




2. list fonts

text2image --text=training00.txt --outputbase=chi --find_fonts --fonts_dir=./fonts --min_coverage=1.0 --render_per_font=false


text2image --text=training01.txt --outputbase=eng --find_fonts --fonts_dir=./fonts --min_coverage=1.0 --render_per_font=false
