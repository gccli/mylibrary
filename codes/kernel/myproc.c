#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h> /* Necessary because we use the proc fs */
#include <asm/uaccess.h>
#include <asm/current.h> // current
#include <linux/sched.h> // current

// Under the RHEL-5.4

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Li Jing");
MODULE_DESCRIPTION("How to use proc filesystem");

#define PROCFS_NAME     "myself"
#define PROCFS_MAX_SIZE 1024

static struct proc_dir_entry *myproc_root;
static struct proc_dir_entry *myproc_file;
static char procfs_buffer[PROCFS_MAX_SIZE];
static long procfs_buffer_size = 0;
static long procfs_isopen;

static ssize_t procfs_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
    static int finished = 0;

    if (finished) {
	printk(KERN_INFO "procfs_read: eof\n");
	finished = 0;
	return 0;
    }
    finished = 1;
    if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size)) {
	return -EFAULT;
    }
    printk(KERN_INFO "procfs_read: read %ld bytes\n", procfs_buffer_size);

    return procfs_buffer_size;
}

static ssize_t
procfs_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
    procfs_buffer_size = len;
    if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
	procfs_buffer_size = PROCFS_MAX_SIZE;
    }

    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size)) {
	return -EFAULT;
    }

    return procfs_buffer_size;
}

static int module_permission(struct inode *inode, int op)
{
    /* 
     * We allow everybody to read from our module, but
     * only root (uid 0) may write to it 
     */
    printk(KERN_INFO "op:%d euid:%d\n", op, current->cred->euid);
    if ((op & 0x4) || ((op & 0x2) && current->cred->euid == 0))
	return 0;

    printk(KERN_INFO "No Permission: access is denied, op:%d\n", op);
    return -EACCES;
}

/* 
 * The file is opened - we don't really care about
 * that, but it does mean we need to increment the
 * module's reference count. 
 */

DECLARE_WAIT_QUEUE_HEAD(WaitQ);
int procfs_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "open: opened:%ld, flags:%d\n", procfs_isopen, file->f_flags);

    if ((file->f_flags & O_NONBLOCK) && procfs_isopen)
	return -EAGAIN;

    try_module_get(THIS_MODULE);

    while(procfs_isopen) {
	int i, is_sig = 0;
	wait_event_interruptible(WaitQ, !procfs_isopen);

	for (i = 0; i < _NSIG_WORDS && !is_sig; i++)
	    is_sig = current->pending.signal.sig[i] & ~current->blocked.sig[i];
	if (is_sig) {
	    module_put(THIS_MODULE);
	    return -EINTR;
	}
    }

    procfs_isopen = 1;

    return 0;
}
/* 
 * The file is closed - again, interesting only because
 * of the reference count. 
 */
int procfs_close(struct inode *inode, struct file *file)
{
    procfs_isopen = 0;
    wake_up(&WaitQ);
    module_put(THIS_MODULE);
    return 0;
}

static struct file_operations proc_fops = {
    .read  = procfs_read,
    .write  = procfs_write,
    .open  = procfs_open,
    .release = procfs_close,
};

static struct inode_operations proc_iops = {
    .permission = module_permission,/* check for permissions */
};


static int __init myprocinit(void)
{
    myproc_root = proc_mkdir("myproc", NULL);
    if (!myproc_root) {
	printk(KERN_ALERT "Error: Could not mkdir /proc/myproc\n");
	return -ENOMEM;
    }
    myproc_file = create_proc_entry(PROCFS_NAME, 0644, myproc_root);
    if (myproc_file == NULL) {
	remove_proc_entry(PROCFS_NAME, myproc_root);
	printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
	return -ENOMEM;
    }

    myproc_file->proc_iops = &proc_iops;
    myproc_file->proc_fops = &proc_fops;
    myproc_file->mode = S_IFREG | S_IRUGO | S_IWUSR;
    myproc_file->uid = 0;
    myproc_file->gid = 0;
    myproc_file->size = 80;

    printk(KERN_INFO "/proc/myproc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit myprocexit(void)
{
    remove_proc_entry(PROCFS_NAME, myproc_root);
    remove_proc_entry("proc/myproc", NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

module_init(myprocinit);
module_exit(myprocexit);
