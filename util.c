#include "util.h"
#include "linux/dcache.h"
#include "linux/inet.h"
#include "linux/limits.h"
#include "linux/printk.h"
#include <linux/in.h>
#include <linux/in6.h>

#define MAX_DENTRIES 256
#define BUF_SIZE 1024
#define IP_SIZE 32

#include "linux/list.h"

int is_pipe(struct file *filp)
{
	return S_ISFIFO(filp->f_path.dentry->d_inode->i_mode);
}

int is_socket(struct file *filp)
{
	return S_ISSOCK(filp->f_path.dentry->d_inode->i_mode);
}

int is_file(struct file *filp)
{
	return S_ISREG(filp->f_path.dentry->d_inode->i_mode);
}

int get_absolute_path(struct file *filp, char *path_buffer)
{
	char *buf = kmalloc(PATH_MAX, GFP_KERNEL);
	memset(buf, 0, GFP_KERNEL);
	char *path = dentry_path_raw(filp->f_path.dentry, buf, PATH_MAX);
	kfree(buf);
	strncpy(path_buffer, path, PATH_MAX);

	return 1;
}

int is_subpath(const char *file_path, const char *dir_path)
{
	size_t file_len = strlen(file_path);
	size_t dir_len = strlen(dir_path);

	if (file_len <= dir_len) {
		return 0;
	}

	if (strncmp(dir_path, file_path, dir_len) == 0) {
		return 1;
	}
	return 0;
}

int validate_ipv4_with_port(const char *ip_with_port, char *valid_buffer)
{
	char ip_str[INET_ADDRSTRLEN];
	unsigned int a, b, c, d;
	int port;
	if (sscanf(ip_with_port, "%15[^:]:%d", ip_str, &port) != 2) {
		return 0; // Invalid format
	}

	if (sscanf(ip_str, "%u.%u.%u.%u", &a, &b, &c, &d) == 4) {
	}
	if (a > 255 || b > 255 || c > 255 || d > 255) {
		pr_err("Invalid ip: >255\n");
		return 0;
	}

	snprintf(valid_buffer, IP_SIZE, "%u.%u.%u.%u:%u", a, b, c, d, port);

	return 1;
}
