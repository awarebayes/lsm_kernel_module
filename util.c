#include "util.h"

#include "linux/list.h"

int is_pipe(struct file *filp)
{
	return S_ISFIFO(filp->f_path.dentry->d_inode->i_mode);
}

int is_socket(struct file *filp)
{
	return S_ISSOCK(filp->f_path.dentry->d_inode->i_mode);
}
