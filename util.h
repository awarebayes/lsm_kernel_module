#include "linux/types.h"
#include <linux/fs.h>
#include <linux/stat.h>

int is_pipe(struct file *filp);
int is_socket(struct file *filp);
int is_file(struct file *filp);
int get_absolute_path(struct file *filp, char *path_buffer);
int is_subpath(const char *file_path, const char *dir_path);
