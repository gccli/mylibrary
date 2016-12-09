#! /bin/bash
fontsdir=/usr/share/fonts/windows

# https://github.com/tesseract-ocr/tesseract/wiki/Training-Tesseract
# http://www.resolveradiologic.com/blog/2013/01/15/training-tesseract/
# http://www.zmonster.me/2015/05/05/tesseract-training.html
rm -f  *.tif *.box *.tr chi.*


echo "--------------------------------------------------------------------------------"
sleep 2
echo

fontname="SimSun"
boxes=""

#for i in $(seq 0 9)
#do
#    text=training0$i.txt
#    outb=chi.$fontname.exp$i
#    text2image --text=$text --outputbase=$outb --font="$fontname" --fonts_dir=$fontsdir
#    [ $? -ne 0 ] && echo "error in text2image" && exit 1
#    tesseract chi.SimSun.exp$i.tif chi.SimSun.exp$i box.train.stderr
#    [ $? -ne 0 ] && echo "error in training" && exit 1

#    boxes="$boxes chi.SimSun.exp$i.box"
#done


eng_fonts="Arial NSimSun SimSun "


function train() {
    text=training.txt
    outb=chi.$fontname.exp0
    text2image --text=$text --outputbase=$outb --font="$fontname" --fonts_dir=$fontsdir
    [ $? -ne 0 ] && echo "error in text2image" && exit 1
    tesseract chi.SimSun.exp0.tif chi.SimSun.exp0 box.train.stderr
    [ $? -ne 0 ] && echo "error in training" && exit 1
    boxes="$boxes chi.SimSun.exp0.box"




    text2image --text=training00.txt --outputbase=training01 --font='' --fonts_dir=./fonts


    text2image --text=training01.txt --outputbase=training01 --font='Times New Roman,' --fonts_dir=./fonts




}

train
echo -e "\n\n---- unicharset_extractor --------------------------"
unicharset_extractor $boxes
[ $? -ne 0 ] && echo "error in unicharset_extractor" && exit 1

echo -e "\n\n---- shapeclustering --------------------------"
shapeclustering -F font_properties -U unicharset *.tr
[ $? -ne 0 ] && echo "error in shapeclustering" && exit 1

echo -e "\n\n---- mftraining --------------------------"
mftraining -F font_properties -U unicharset -O chi.unicharset *.tr
[ $? -ne 0 ] && echo "error in mftraining" && exit 1

echo -e "\n\n---- cntraining --------------------------"
cntraining *.tr
[ $? -ne 0 ] && echo "error in cntraining" && exit 1

echo -e "\n\n---- combine_tessdata --------------------------"
# shapetable, normproto, inttemp, pffmtable, unicharset
mv unicharset chi.unicharset
mv shapetable chi.shapetable
mv inttemp chi.inttemp
mv pffmtable chi.pffmtable
mv normproto chi.normproto

combine_tessdata chi
