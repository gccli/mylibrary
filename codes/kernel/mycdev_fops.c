#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/uaccess.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Jing");
MODULE_DESCRIPTION("This a character device driver");

static int   mycdevopen;
static char  mycdevmsg[256];
static char *mycdevptr;
int mycdev_open(struct inode *inode, struct file *file)
{
    static int counter = 0;
    if (mycdevopen)
	return -EBUSY;
    
    mycdevopen++;
    sprintf(mycdevmsg, "I already told you %d times Hello world!\n", counter++);
    mycdevptr = mycdevmsg;
    try_module_get(THIS_MODULE);
    
    return 0;
}

int mycdev_release(struct inode *inode, struct file *file)
{
    mycdevopen--;
    module_put(THIS_MODULE);
    return 0;
}

ssize_t mycdev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    if (*mycdevptr == 0)
	return 0;
    
    while (length && *mycdevptr) {
	/* 
	 * The buffer is in the user data segment, not the kernel 
	 * segment so "*" assignment won't work.  We have to use 
	 * put_user which copies data from the kernel data segment to
	 * the user data segment. 
	 */
	put_user(*(mycdevptr++), buffer++);
	length--;
	bytes_read++;
    }
    
    return bytes_read;
}

ssize_t mycdev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
    return -EINVAL;
}
