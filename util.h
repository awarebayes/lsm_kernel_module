#include "linux/types.h"
#include <linux/fs.h>
#include <linux/stat.h>

#define NIPQUAD(addr)                                                          \
	((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1],              \
		((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3]

int is_pipe(struct file *filp);
int is_socket(struct file *filp);
int is_file(struct file *filp);
int get_absolute_path(struct file *filp, char *path_buffer);
int is_subpath(const char *file_path, const char *dir_path);
int validate_ipv4_with_port(const char *ip_with_port, char *valid_buffer);
