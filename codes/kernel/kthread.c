#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kthread.h>

static struct task_struct *mytask;

int kthreadfunc(void *param)
{
    printk(KERN_INFO "kernel thread running ...\n");

    int i;
    for(i=0; i<1000; ++i)
    {
	printk(KERN_INFO "kernel thread %d\n", i);
    }
    while(i)
	;


    return 0;
}

static int __init kthread_init(void)
{
    mytask = kthread_run(kthreadfunc, NULL, "mythread");

    return 0;
}

static void __exit kthread_exit(void)
{
    printk(KERN_INFO "module release\n");
}

module_init(kthread_init)
module_exit(kthread_exit)
MODULE_LICENSE("GPL");
