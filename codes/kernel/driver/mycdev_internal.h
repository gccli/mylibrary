#ifndef MYCDEV_INTERNAL_H__
#define MYCDEV_INTERNAL_H__


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/uaccess.h> 

#include "mycdev.h"




int mycdev_open(struct inode *, struct file *);
int mycdev_release(struct inode *, struct file *);
ssize_t mycdev_read(struct file *, char *, size_t, loff_t *);
ssize_t mycdev_write(struct file *, const char *, size_t, loff_t *);
//int mycdev_ioctl(struct file *, unsigned int, unsigned long);
int mycdev_ioctl(struct inode *inode, struct file *file, unsigned int inum, unsigned long iparam);
#endif
