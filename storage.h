#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>

#define PIPE_R_SIZE 256
#define NUM_RESTRICTIONS 16

struct pipe_restriction {
	char pipename[PIPE_R_SIZE];
};

struct restriction {
	struct list_head list;
	pid_t pid;

	int num_allowed_pipes;
	struct pipe_restriction pipe_restrictions[NUM_RESTRICTIONS];
};

void debug_print_restrictions(void);
int add_restriction(pid_t process);
int remove_restriction(pid_t process);
struct restriction *get_restricted_process(pid_t process);

int restriction_allow_pipe(pid_t process, char *pipename);
int restriction_has_pipe_allowed(pid_t process, char *pipename);
