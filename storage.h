#include "linux/limits.h"
#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>

#define FILE_R_SIZE PATH_MAX
#define NUM_RESTRICTIONS 16
#define IP_SIZE 32
#define RETURN_BUFFER_SIZE 16192

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

struct ip_restriction {
	char ip_str[IP_SIZE];
};

struct restriction {
	struct list_head list;
	pid_t pid;

	int num_allowed_files;
	struct file_restriction file_restrictions[NUM_RESTRICTIONS];

	int num_allowed_ips;
	struct ip_restriction ip_restrictions[NUM_RESTRICTIONS];

	int dir_restricted;
	char working_dir[PATH_MAX];
};

void debug_print_restrictions(void);
int add_restriction(pid_t process);
int remove_restriction(pid_t process);
struct restriction *get_restricted_process(pid_t process);

int restriction_allow_file(pid_t process, char *pipename, enum file_type ft);
int restriction_allow_directory(pid_t process, char *dirname);
int restriction_allow_ip(pid_t process, char *ip_str);

int restriction_has_file_allowed(pid_t process, char *pipename,
				 enum file_type ft);
int restriction_has_ip_allowed(pid_t process, char *ip_str);

const char *return_buffer_get(void);
int return_buffer_set(const char *str);

void print_csv_restricted_pids(void);
void print_csv_restriction_info(pid_t process);
