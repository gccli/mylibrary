#! /bin/bash
fontsdir=/usr/share/fonts/windows

# https://github.com/tesseract-ocr/tesseract/wiki/Training-Tesseract
# http://www.resolveradiologic.com/blog/2013/01/15/training-tesseract/
# http://www.zmonster.me/2015/05/05/tesseract-training.html
rm -f  *.tif *.box *.tr chi.*


sleep 2
echo "--------------------------------------------------------------------------------"
echo

fontname="SimSun"
boxes=""

for i in $(seq 0 9)
do
    text=training0$i.txt
    outb=chi.$fontname.exp$i
    text2image --text=$text --outputbase=$outb --font="$fontname" --fonts_dir=$fontsdir
    [ $? -ne 0 ] && echo "error in text2image" && exit 1
    tesseract chi.SimSun.exp$i.tif chi.SimSun.exp$i box.train.stderr
    [ $? -ne 0 ] && echo "error in training" && exit 1

    boxes="$boxes chi.SimSun.exp$i.box"
done

unicharset_extractor $boxes
[ $? -ne 0 ] && echo "error in unicharset_extractor" && exit 1

shapeclustering -F font_properties -U unicharset *.tr
[ $? -ne 0 ] && echo "error in shapeclustering" && exit 1

mftraining -F font_properties -U unicharset -O chi.unicharset *.tr
[ $? -ne 0 ] && echo "error in mftraining" && exit 1

cntraining *.tr
[ $? -ne 0 ] && echo "error in cntraining" && exit 1

# shapetable, normproto, inttemp, pffmtable, unicharset
mv unicharset chi.unicharset
mv shapetable chi.shapetable
mv inttemp chi.inttemp
mv pffmtable chi.pffmtable
mv normproto chi.normproto

combine_tessdata chi
