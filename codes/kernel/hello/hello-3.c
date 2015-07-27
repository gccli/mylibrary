#include <linux/module.h>/* Needed by all modules */
#include <linux/kernel.h>/* Needed for KERN_INFO */
#include <linux/init.h>/* Needed for KERN_INFO */
#include <linux/moduleparam.h>
#include <linux/stat.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Jing");
MODULE_DESCRIPTION("Just a exercise");

static short myshort = 100;
static int   myint = 99;
static long  mylong = 98;
static char* mystr = "hello";
static int   myarray[100];
static int   myarrayc = 100;


module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
module_param(mylong, long, S_IRUSR);
module_param(mystr, charp, 0000);
module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, "A short integer");
MODULE_PARM_DESC(myint, "An integer");
MODULE_PARM_DESC(mylong, "A long integer");
MODULE_PARM_DESC(mystr, "A character string");

module_param_array(myarray, int, &myarrayc, 0000);
MODULE_PARM_DESC(myintArray, "An array of integers");

static int __init hello2init(void)
{
    int i;
    int off=0;
    char temp[1024] = {0};
    for (i=0; i<myarrayc; ++i)
	off += sprintf(temp+off, "%d ", myarray[i]);

    printk(KERN_INFO "short  %d ", myshort);
    printk(KERN_INFO "int    %d ", myint);
    printk(KERN_INFO "long   %ld ", mylong);
    printk(KERN_INFO "string %s ", mystr);
    printk(KERN_INFO "array  %s ", temp);

    printk(KERN_INFO "\nMoudle Initialized\n");
    return 0;
}

static void __exit hello2exit(void)
{
    printk(KERN_INFO "Moudle Deinitialize\n");
}

module_init(hello2init);
module_exit(hello2exit);



