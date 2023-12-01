#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>

#define FILE_R_SIZE 256
#define NUM_RESTRICTIONS 16

enum file_type {
	ALLOWED_FILE_REGULAR,
	ALLOWED_FILE_UNIX_SOCKET,
	ALLOWED_FILE_PIPE,
	_FILE_TYPE_UNKNOWN,
};

struct file_restriction {
	char filename[FILE_R_SIZE];
	enum file_type type;
};

struct restriction {
	struct list_head list;
	pid_t pid;

	int num_allowed_files;
	struct file_restriction file_restrictions[NUM_RESTRICTIONS];
};

void debug_print_restrictions(void);
int add_restriction(pid_t process);
int remove_restriction(pid_t process);
struct restriction *get_restricted_process(pid_t process);

int restriction_allow_file(pid_t process, char *pipename, enum file_type ft);
int restriction_has_file_allowed(pid_t process, char *pipename,
				 enum file_type ft);
