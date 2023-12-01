#include "linux/limits.h"
#include <linux/security.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/lsm_hooks.h>
#include <linux/kern_levels.h>
#include <linux/binfmts.h>
#include <linux/sched.h>
#include <linux/in.h>
#include <linux/in6.h>

#include "hooks.h"
#include "util.h"
#include "storage.h"

int my_security_socket_connect(struct socket *sock, struct sockaddr *addr,
			       int addrlen)
{
	pid_t current_pid = current->pid;
	struct restriction *r = get_restricted_process(current_pid);
	if (r == NULL) {
		return 0; // process is not restricted
	}

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

int my_security_file_open(struct file *file)
{
	pid_t current_pid = current->pid;
	struct restriction *r = get_restricted_process(current_pid);
	if (r == NULL) {
		return 0; // process is not restricted
	}

	char abspath[PATH_MAX] = { 0 };
	get_absolute_path(file, abspath);
	pr_err("file '%s' was accessed by a monitored process with PID: %d\n",
	       abspath, current_pid);

	enum file_type ft = _FILE_TYPE_UNKNOWN;
	if (is_socket(file)) {
		ft = ALLOWED_FILE_UNIX_SOCKET;
	} else if (is_pipe(file)) {
		ft = ALLOWED_FILE_PIPE;
	} else if (is_file(file)) {
		ft = ALLOWED_FILE_REGULAR;
	}
	if (restriction_allow_file(current_pid, abspath, ft)) {
		return 0;
	}
	return -EACCES;
}
