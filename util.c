#include "util.h"

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
	struct dentry *dentry;

	if (!filp || !filp->f_path.dentry) {
		printk(KERN_ERR "Invalid file pointer or dentry.\n");
		return 1;
	}

	struct dentry *dentries[MAX_DENTRIES];
	struct dentry *dentries_inversed[MAX_DENTRIES];

	dentry = filp->f_path.dentry;

	int dentries_found = 0;

	while (dentry) {
		if (dentry == dentry->d_parent && dentry->d_name.len == 1 &&
		    dentry->d_name.name[0] == '/')
			break;
		dentries[dentries_found] = dentry;
		dentries_found++;
		dentry = dentry->d_parent;
	}

	for (int i = 0; i < dentries_found; i++) {
		dentries_inversed[i] = dentries[dentries_found - i - 1];
	}

	int buffer_index = 0;
	for (int i = 0; i < dentries_found; i++) {
		int len = dentries_inversed[i]->d_name.len;
		if (buffer_index + len + 1 < PATH_MAX) {
			buffer_index +=
				sprintf(&path_buffer[buffer_index], "/%.*s",
					len, dentries_inversed[i]->d_name.name);
		} else {
			printk(KERN_ERR "Path exceeds buffer size.\n");
			return 0;
		}
	}

	return 1;
}
