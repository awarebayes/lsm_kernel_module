#include "linux/types.h"
#include <linux/fs.h>
#include <linux/stat.h>

int is_pipe(struct file *filp);
int is_socket(struct file *filp);
