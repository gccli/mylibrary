#include <NLPIR.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <getopt.h>

#include "utiltime.h"
#include "utilfile.h"

void AnalyzeFile(const char *file)
{
    double start;
    printf("Analyze \"%s\", result store to /tmp/result.txt\n", file);

    start = timing_start();
    if (!NLPIR_FileProcess(file,"/tmp/result.txt")) {
        printf("error in file process - %s\n", NLPIR_GetLastErrorMsg());
    };
    printf("File bytes: %f kB time cost %lf\n",
           get_file_size(file)/1024.0, timing_cost(start));
    printf("KeyWords: %s ...\n", NLPIR_GetFileKeyWords(file, 20));
    printf("NewWords: %s ...\n", NLPIR_GetFileNewWords(file, 20));


    NLPIR_Exit();

    exit(0);
}

int main(int argc, char *argv[])
{
    int c;
    int opt_add_usrdict = 0;
    int opt_del_userdict = 0;
    int opt_save_userdict = 0;
    char *str_word = NULL;

    // Initialize the library
    if(!NLPIR_Init(NULL, UTF8_CODE)) {
        printf("ICTCLAS INIT FAILED!\n");
        return 1;
    }

    while ((c = getopt(argc, argv, "f:a:d:s")) != -1) {
        switch(c) {
        case 'f':
            AnalyzeFile(optarg);
            break;
        case 'a':
            opt_add_usrdict = 1;
            str_word = strdup(optarg);
            break;
        case 'd':
            opt_del_userdict = 1;
            str_word = strdup(optarg);
            break;
        case 's':
            opt_save_userdict = 1;
            break;
        default:
            break;
        }
    }

    int n = 0;
    char word[128] = {0};
    const char *text = "现有的分词算法可分为三大类："
        "基于字符串匹配的分词方法、"
        "基于理解的分词方法和基于统计的分词方法."
        "按照是否与词性标注过程相结合，"
        "又可以分为单纯分词方法和分词与标注相结合的一体化方法。";
    bool with_wight = false;
    printf("Original Result:\n");
    printf("--------------------------------\n");
    printf(" %s\n", NLPIR_ParagraphProcess(text));
    printf(" Keywords: %s\n", NLPIR_GetKeyWords(text, 10, with_wight));
    printf(" FingerPrint:0x%lx\n", NLPIR_FingerPrint(text));
    printf(" New words: %s\n", NLPIR_GetNewWords(text));
    printf("\n");
    if (opt_add_usrdict && NLPIR_AddUserWord(str_word)) {
        printf("After add user dict Result: \n%s\n\n", NLPIR_ParagraphProcess(text));
    }
    if (opt_del_userdict && NLPIR_DelUsrWord(str_word)) {
        printf("After add user dict Result: \n%s\n\n", NLPIR_ParagraphProcess(text));
    }
    if (opt_save_userdict && !NLPIR_SaveTheUsrDic()) {
        printf("Save user dict failed\n");
        return 3;
    }


    const result_t *r = NLPIR_ParagraphProcessA(text, &n);
    printf ("Got %d words, word count %d\n", n, NLPIR_GetParagraphProcessAWordCount(text));

    for(int i=0; i<n; ++i) {
        const char *whichdict="";
        switch (r[i].word_type) {
        case 0:
            whichdict = "核心词典";
            break;
        case 1:
            whichdict = "用户词典";
            break;
        case 2:
            whichdict = "专业词典";
            break;
        default:
            break;
        }
        if (i+1 == 15) {
            printf("continue (y/n) ");
            if (getchar() != (int) 'y') break;
        }

        strncpy(word,text+r[i].start,r[i].length);
        word[r[i].length]=0;
        printf("No.%d: start:%d, length:%d, POS_ID:%d, "
               "Word_ID:%d, UserDefine:%s, Word:%s, Weight:%d\n",
               i+1, r[i].start, r[i].length, r[i].iPOS, r[i].word_ID,
               whichdict, word, r[i].weight);
    }

    NLPIR_Exit();
    return 0;
}
