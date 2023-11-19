#include <linux/fs.h>
#include <linux/security.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/lsm_hooks.h>
#include <linux/kern_levels.h>
#include <linux/binfmts.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/in.h>
#include <linux/in6.h>

#include "hooks.h"

int my_security_socket_connect(struct socket *sock, struct sockaddr *addr,
			       int addrlen)
{
	pid_t current_pid = current->pid;
	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *saddr = (struct sockaddr_in *)addr;
		printk(KERN_INFO
		       "Connected to IPv4 address: %pI4:%d by PID %d\n",
		       &saddr->sin_addr, ntohs(saddr->sin_port), current_pid);
	} else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)addr;
		printk(KERN_INFO
		       "Connected to IPv6 address: %pI6:%d by PID %d\n",
		       &saddr->sin6_addr, ntohs(saddr->sin6_port), current_pid);
	}

	return 0;
}

static inline int is_pipe(struct file *filp)
{
	return S_ISFIFO(filp->f_path.dentry->d_inode->i_mode);
}

static inline int is_socket(struct file *filp)
{
	return S_ISSOCK(filp->f_path.dentry->d_inode->i_mode);
}

int my_security_file_open(struct file *file)
{
	const char *path = file->f_path.dentry->d_name.name;
	pid_t current_pid = current->pid;
	if (is_socket(file)) {
		printk(KERN_INFO
		       "File socket <<%s>> accessed by process with PID: %d\n",
		       path, current_pid);
	}

	if (is_pipe(file)) {
		printk(KERN_INFO
		       "Pipe <<%s>> accessed by process with PID: %d\n",
		       path, current_pid);
	}
	return 0;
}
