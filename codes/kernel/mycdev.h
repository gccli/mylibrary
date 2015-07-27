#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/uaccess.h> 

int mycdev_open(struct inode *, struct file *);
int mycdev_release(struct inode *, struct file *);
ssize_t mycdev_read(struct file *, char *, size_t, loff_t *);
ssize_t mycdev_write(struct file *, const char *, size_t, loff_t *);
