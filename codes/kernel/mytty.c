#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tty.h>

static void print_string(char *str)
{
    struct tty_struct *my_tty;

    if ((my_tty = current->signal->tty) != NULL) {
	my_tty->driver->ops->write (my_tty, str, strlen(str));
	my_tty->driver->ops->write (my_tty, "\015\012", 2);
    }
}

static int __init print_string_init(void)
{
    char str[128] = {0};
    sprintf(str, "The module has been inserted.  Hello world! PID:%d\n", current->pid);
    print_string(str);
    return 0;
}

static void __exit print_string_exit(void)
{
    print_string("The module has been removed.  Farewell world!");
}

module_init(print_string_init);
module_exit(print_string_exit);
