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

#define BUF_SIZE 256

int my_security_socket_connect(struct socket *sock, struct sockaddr *addr,
			       int addrlen)
{
	pid_t current_pid = current->pid;
	struct restriction *r = get_restricted_process(current_pid);
	if (r == NULL) {
		return 0; // process is not restricted
	}

	if (addr->sa_family == AF_INET) {
		char ip_str_buffer[BUF_SIZE] = { 0 };
		struct sockaddr_in *saddr = (struct sockaddr_in *)addr;

		snprintf(ip_str_buffer, IP_SIZE, "%d.%d.%d.%d:%d",
			 NIPQUAD(saddr->sin_addr), ntohs(saddr->sin_port));
		pr_info("Checking ip '%s' against %d's allowed ips\n",
			ip_str_buffer, current_pid);
		if (restriction_has_ip_allowed(current_pid, ip_str_buffer)) {
			pr_info("Connection allowed");
			return 0;
		} else {
			return -EPERM;
		}
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
	pr_info("file '%s' was accessed by a monitored process with PID: %d\n",
		abspath, current_pid);

	if (is_subpath(abspath, "/lib/")) {
		return 0; // allow lib
	}

	if (is_subpath(abspath, "/usr/")) {
		return 0; // allow usr
	}

	if (r->dir_restricted) {
		if (is_subpath(abspath, r->working_dir)) {
			pr_info("Restricted pid %d was trying to access file %s inside its working dir: %s, access ALLOWED\n",
				current_pid, abspath, r->working_dir);
			return 0;
		} else {
			pr_info("Restricted pid %d was trying to access file %s outside its working dir: %s\n",
				current_pid, abspath, r->working_dir);
		}
	}

	enum file_type ft = _FILE_TYPE_UNKNOWN;
	if (is_socket(file)) {
		ft = ALLOWED_FILE_UNIX_SOCKET;
	} else if (is_pipe(file)) {
		ft = ALLOWED_FILE_PIPE;
	} else if (is_file(file)) {
		ft = ALLOWED_FILE_REGULAR;
	}
	if (restriction_has_file_allowed(current_pid, abspath, ft)) {
		pr_info("Access to file '%s' allowed because it was explicitly added.\n",
			abspath);
		return 0;
	}
	if (r->dir_restricted) {
		pr_info("Access to file %s forbidden beucause it was not in restricted directory and was not found in explicitly added files\n",
			abspath);
	} else {
		pr_info("Access to file %s forbidden beucause it was not found in explicitly added files\n",
			abspath);
	}
	return -EACCES;
}
