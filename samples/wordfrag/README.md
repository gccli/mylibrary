<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />

中文分词库NLPIR调研
===============

NLPIR汉语分词系统(又名ICTCLAS2013),主要功能包括中文分词；词性标注；命名实体识别；用户词典功能；支持GBK编码、UTF8编码、BIG5编码。新增微博分词、新词发现与关键词提取；张华平博士先后倾力打造十余年，内核升级10次。汉语分词牵涉到汉语分词、未定义词识别、词性标注以及语言特例等多个因素，大多数系统缺乏统一的处理方法，往往采用松散耦合的模块组合方式，最终模型并不能准确有效地表达千差万别的语言现象，而ICTCLAS采用了_层叠隐马尔可夫模型_（Hierarchical Hidden Markov Model），将汉语词法分析的所有环节都统一到了一个完整的理论框架中，获得最好的总体效果，相关理论研究发表在顶级国际会议和杂志上，从理论上和实践上都证实了该模型的先进性


库的使用
------
NLPIR库提供了C API,在调用库之前必须通过通过`NLPIR_Init`初始化，主要是指定NLPIR的工作目录和编码格式，如UTF8。工作目录下必须存在Data目录，此目录中含有NLPIR所需的词典及配置文件。<br/>

    int NLPIR_Init(const char * sDataPath=0,int encode=GBK_CODE,const char*sLicenceCode=0);

若要分析文件，可调用文件相关接口`NLPIR_FileProcess`，该函数将结果存入一个文件。<br/>
若要分析内存，可调用`NLPIR_ParagraphProcess`或`NLPIR_ParagraphProcessA`，两者都是将分析结果通过指针返回，只是返回结构有所不同；前都返回flat buffer，后者返回结构体数组，包括词所在文本中的位置，长度等信息，结果包括词性标注等内类，如下所示：<br/>

```
现有/v 的/ude1 分词/v 算法/n 可/v 分为/v 三/m 大/a 类/n ：/wp 基于/p 字符串/n 匹配/vi 的/ude1 分词/v 方法/n 、/wn 基于/p 理解/v 的/ude1 分词/v 方法/n 和/cc 基于/p 统计/v …
```

另外，正如NLPIR所介绍的那样，NLPIR还提供新词发现，关键词提取，指纹提取等功能，通过以下函数实现

    NLPIR_GetKeyWords(text, 10, with_wight)
    NLPIR_FingerPrint(text)
    NLPIR_GetNewWords(text)



用户词典功能
------

词典库可能是我们比较关注的，用户词典有两种添加方式：</br>
1.文件方式，将用户词典每行给一个词，如果需要添加词性，可以再后面用空格分开，如下所示：

     自然语言处理 nz
     大数据 nz

调用函数为：</br>
`nCount = NLPIR_ImportUserDict("userdic.txt");`

2.内存方式，需要增加用户词典的时候，调用下面的语句如：<br/>
    NLPIR_AddUserWord("计算机学院  xueyuan");
计算机学院是用户词，后面的是自定义的词性标签。


性能
--

在一个内存为4G的虚拟机下处理一个3M多的文件，用时4秒多

    Analyze "/home/lijing/tmp/samples/orig/cn/8859.txt_utf8", result store to /tmp/result.txt
    File bytes: 3372.133789 kB time cost 4.166020

CPU信息如下：

    grep 'model name' /proc/cpuinfo
    model name	: Intel(R) Core(TM) i7-4790 CPU @ 3.60GHz
    model name	: Intel(R) Core(TM) i7-4790 CPU @ 3.60GHz


优点与不足
-----

- 分析准确度高
- 相对基于统计和算法，速度比较快
- 代码接口丰富，提供C/C++接口，易于集成使用
- 能够自动发现新词

不足

- 需要维护词典库，如增加新词
