#include "mycdev_internal.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Jing");
MODULE_DESCRIPTION("This a character device driver");

static struct file_operations fops = {
    .read = mycdev_read,
    .write = mycdev_write,
    .open = mycdev_open,
    .release = mycdev_release,
//    .compat_ioctl = mycdev_ioctl
    .ioctl = mycdev_ioctl
};


static int __init mycdevinit(void)
{
    int major;

    printk(KERN_INFO "Register device %s ...\n", DEVICE_NAME);
    if ((major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops)) < 0) {
	printk(KERN_ALERT "Registering char device failed with %d\n", major);
	return major;
    }

    printk(KERN_INFO "Register device successfully with major %d\n", MAJOR_NUM);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "  'mknod /dev/%s c %d 0'\n", DEVICE_NAME, MAJOR_NUM);
    printk(KERN_INFO "Try various minor numbers. Try to cat and echo to the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return 0;
}

static void __exit mycdevexit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_ALERT "Unregister mycdev\n");
}

module_init(mycdevinit);
module_exit(mycdevexit);
