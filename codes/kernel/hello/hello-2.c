#include <linux/module.h>/* Needed by all modules */
#include <linux/kernel.h>/* Needed for KERN_INFO */
#include <linux/init.h>/* Needed for KERN_INFO */

// __init macro causes the init function to be discarded and its memory freed once the init function finishes for built-in drivers, but not loadable modules. 

static int myvar = 100;
static int __initdata idata = 3;
static int __init hello2init(void)
{
    printk(KERN_INFO "Hello world, my variable is %d\n", myvar);
    printk(KERN_INFO "Hello world, idata %d\n", idata);
    return 0;
}


static void __exit hello2exit(void)
{
    printk(KERN_INFO "Goodbye world 2.\n");
}

module_init(hello2init);
module_exit(hello2exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Jing");
MODULE_DESCRIPTION("Just a exercise");
