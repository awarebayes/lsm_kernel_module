#include "util.h"
#include "linux/dcache.h"
#include "linux/limits.h"

#define MAX_DENTRIES 256

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
