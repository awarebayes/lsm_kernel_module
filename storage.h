#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>

#define BUFFER_SIZE 256
#define NUM_RESTRICTIONS 16

struct pipe_restriction {
	char pipename[BUFFER_SIZE];
};

struct restriction {
	struct list_head list;
	pid_t pid;
	int num_restricted_pipes;
	struct pipe_restriction pipe_restrictions[NUM_RESTRICTIONS];
};

struct restriction *get_restricted_process(pid_t process);
int add_restriction(pid_t process);
int remove_restriction(pid_t process);
