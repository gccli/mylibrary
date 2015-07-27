#include "mycdev_internal.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Jing");
MODULE_DESCRIPTION("This a character device driver");

#define BUFLEN 256

static int   mycdevopen;
static char  mycdevmsg[BUFLEN];
static char *mycdevptr;
int mycdev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "device open (%p)\n", file);
    if (mycdevopen)
	return -EBUSY;
    
    mycdevopen++;
    mycdevptr = mycdevmsg;
    try_module_get(THIS_MODULE);
    
    return 0;
}

int mycdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "device release(%p,%p)\n", inode, file);

    mycdevopen--;
    module_put(THIS_MODULE);
    return 0;
}

ssize_t mycdev_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
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
    printk(KERN_INFO "device read %d bytes, %d left\n", bytes_read, length);

    return bytes_read;
}

ssize_t mycdev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
    int i;

    printk(KERN_INFO "device write(%p,%s,%d)\n", filp, buffer, length);
    for (i = 0; i < length && i < BUFLEN; i++)
	get_user(mycdevmsg[i], buffer + i);
    mycdevptr = mycdevmsg;

    return 0;
}


/* 
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */
int mycdev_ioctl(struct inode *inode, struct file *file, unsigned int inum, unsigned long iparam)
{
    int i;
    char *temp;
    char ch;

    printk(KERN_INFO "device ioctl(%p,%d,%ld)\n", file, inum, iparam);
    switch (inum) {
	case MYCDEV_WRITE:
	    /* 
	     * Receive a pointer to a message (in user space) and set that
	     * to be the device's message.  Get the parameter given to 
	     * ioctl by the process. 
	     */
	    temp = (char *)iparam;

	    /* 
	     * Find the length of the message 
	     */
	    get_user(ch, temp);
	    for (i = 0; ch && i < BUFLEN; i++, temp++)
		get_user(ch, temp);

	    mycdev_write(file, (char *)iparam, i, 0);
	    break;

	case MYCDEV_READ:
	    /* 
	     * Give the current message to the calling process - 
	     * the parameter we got is a pointer, fill it. 
	     */
	    i = mycdev_read(file, (char *)iparam, 99, 0);

	    /* 
	     * Put a zero at the end of the buffer, so it will be 
	     * properly terminated 
	     */
	    put_user('\0', (char *)iparam + i);
	    break;

	case MYCDEV_NTHBYTE:
	    /* 
	     * This ioctl is both input (iparam) and 
	     * output (the return value of this function) 
	     */
	    return mycdevmsg[iparam];
	    break;
    }

    return 0;
}
