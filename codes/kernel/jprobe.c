#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

/* Proxy routine having the same arguments as actual sys_fork() routine */
asmlinkage long jsys_unlink(const char __user *filename)
{
    printk(KERN_INFO "jprobe: file '%s' will be removed\n", filename);
    jprobe_return();

    return 0;
}

static struct jprobe my_jprobe = {
    .entry = jsys_unlink,
};

static int __init jprobe_init(void)
{
    int ret;
    my_jprobe.kp.symbol_name = "sys_unlink";

    ret = register_jprobe(&my_jprobe);
    if (ret < 0) {
	printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
	return -1;
    }
    printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	   my_jprobe.kp.addr, my_jprobe.entry);
    return 0;
}

static void __exit jprobe_exit(void)
{
    unregister_jprobe(&my_jprobe);
    printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
