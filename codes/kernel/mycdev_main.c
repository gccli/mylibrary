#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/uaccess.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Jing");
MODULE_DESCRIPTION("This a character device driver");

#define DEVICE_NAME "mycdev" // Dev name as it appears in /proc/devices

int mycdev_major;

int mycdev_open(struct inode *, struct file *);
int mycdev_release(struct inode *, struct file *);
ssize_t mycdev_read(struct file *, char *, size_t, loff_t *);
ssize_t mycdev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .read = mycdev_read,
    .write = mycdev_write,
    .open = mycdev_open,
    .release = mycdev_release
};

static int __init mycdevinit(void)
{
    printk(KERN_INFO "Register device %s ...\n", DEVICE_NAME);

    if ((mycdev_major = register_chrdev(0, DEVICE_NAME, &fops)) < 0) {
	printk(KERN_ALERT "Registering char device failed with %d\n", mycdev_major);
	return mycdev_major;
    }

    printk(KERN_INFO "Register device successfully with mycdev_major %d\n", mycdev_major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "  'mknod /dev/%s c %d 0'\n", DEVICE_NAME, mycdev_major);
    printk(KERN_INFO "Try various minor numbers. Try to cat and echo to the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return 0;
}

static void __exit mycdevexit(void)
{
    unregister_chrdev(mycdev_major, DEVICE_NAME);
    printk(KERN_ALERT "Unregister mycdev\n");
}

module_init(mycdevinit);
module_exit(mycdevexit);
