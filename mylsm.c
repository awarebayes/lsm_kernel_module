#include "linux/printk.h"
#include <linux/lsm_hooks.h>
#include <linux/kern_levels.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/types.h>

#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/types.h>
#include <linux/timer.h>

#include "hooks.h"
#include "commands.h"

#define MAX_BUF_SIZE PAGE_SIZE
#define MAX_NUM_WATCHES 2048
static unsigned long timeout_duration = 2000;

static struct proc_dir_entry *fortune_dir, *fortune_file;
static char *buffer = NULL;
int working = false;

#define FORTUNE_DIRNAME "mylsm"
#define FORTUNE_FILENAME "mylsm"
#define FORTUNE_PATH FORTUNE_DIRNAME "/" FORTUNE_FILENAME
static struct timer_list my_timer;

#define KERN_LOG_MSG()                                                         \
	{                                                                      \
		printk(KERN_INFO "FORTUNE_MODULE: %s called.\n", __func__);    \
	}
#define KERN_ERR_MSG(err)                                                      \
	{                                                                      \
		printk(KERN_ERR "FORTUNE_MODULE: %s.\n", err);                 \
	}
#define KERN_INFO_MSG(msg)                                                     \
	{                                                                      \
		printk(KERN_INFO "FORTUNE_MODULE: %s.\n", msg);                \
	}

static void *ct_seq_start(struct seq_file *m, loff_t *pos)
{
	KERN_LOG_MSG();

	if (*pos == 0)
		return buffer;

	*pos = 0;
	return NULL;
}

static void *ct_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	KERN_LOG_MSG();

	if (buffer == NULL)
		return NULL;

	(*pos)++;
	printk(KERN_INFO "mylfs: Handling command: v = '%s', pos = %Ld.\n",
	       (char *)v, *pos);

	return NULL;
}

static void ct_seq_stop(struct seq_file *m, void *v)
{
}

static int ct_seq_show(struct seq_file *sfile, void *v)
{
	return 0;
}

const struct seq_operations seq_ops = {
	start: ct_seq_start,
	next: ct_seq_next,
	stop: ct_seq_stop,
	show: ct_seq_show,
};

static int fortune_open(struct inode *sp_inode, struct file *sp_file)
{
	KERN_LOG_MSG();
	return seq_open(sp_file, &seq_ops);
}

static int fortune_release(struct inode *sp_node, struct file *sp_file)
{
	KERN_LOG_MSG();
	return 0;
}

static ssize_t fortune_read(struct file *file, char __user *buf, size_t len,
			    loff_t *f_pos)
{
	KERN_LOG_MSG();
	return seq_read(file, buf, len, f_pos);
}

static ssize_t fortune_write(struct file *file, const char __user *buf,
			     size_t len, loff_t *ppos)
{
	KERN_LOG_MSG();

	if (len > MAX_BUF_SIZE) {
		KERN_ERR_MSG("Buffer overflow");
		return -ENOSPC;
	}

	if (copy_from_user(buffer, buf, len) != 0) {
		KERN_ERR_MSG("copy_from_user function get a error");
		return -EFAULT;
	}

	buffer[len - 1] = '\0';
	pr_info("Handling command %s", buffer);
	parse_command(buffer);

	return len;
}

static const struct proc_ops fops = {
	proc_write: fortune_write,
	proc_read: fortune_read,
	proc_open: fortune_open,
	proc_release: fortune_release,
};

static void cleanup_fortune(void)
{
	KERN_LOG_MSG();

	if (fortune_file != NULL)
		remove_proc_entry(FORTUNE_FILENAME, fortune_dir);

	if (fortune_dir != NULL)
		remove_proc_entry(FORTUNE_DIRNAME, NULL);

	kfree(buffer);
}

static int setup_proc(void)
{
	KERN_LOG_MSG();

	if ((buffer = kvzalloc(MAX_BUF_SIZE, GFP_KERNEL)) == NULL) {
		KERN_ERR_MSG("Allocate memory error.");
		return -ENOMEM;
	}

	if ((fortune_dir = proc_mkdir(FORTUNE_DIRNAME, NULL)) == NULL) {
		KERN_ERR_MSG("Error during create directory in proc");
		cleanup_fortune();
		return -ENOMEM;
	}

	if ((fortune_file = proc_create(FORTUNE_FILENAME, S_IRUGO | S_IWUGO,
					fortune_dir, &fops)) == NULL) {
		KERN_ERR_MSG("Error during create file in proc");
		cleanup_fortune();
		return -ENOMEM;
	}

	KERN_INFO_MSG("Module has been successfully inited.\n");
	return 0;
}

static struct security_hook_list mylsm_hooks[] __lsm_ro_after_init = {
	LSM_HOOK_INIT(socket_connect, my_security_socket_connect),
	LSM_HOOK_INIT(file_open, my_security_file_open),
};

static void timer_callback(struct timer_list *timer)
{
	pr_info("Timeout occurred!\n");
	setup_proc();
	del_timer(&my_timer);
}

static int __init mylsm_init(void)
{
	security_add_hooks(mylsm_hooks, ARRAY_SIZE(mylsm_hooks), "mylsm");
	timer_setup(&my_timer, timer_callback, 0);
	my_timer.expires = jiffies + msecs_to_jiffies(timeout_duration);
	add_timer(&my_timer);
	return 0;
}

DEFINE_LSM(mylsm) = {
	.name = "mylsm",
	.init = mylsm_init,
};
