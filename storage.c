#include "storage.h"
#include "linux/list.h"
#include "linux/printk.h"
#include "linux/slab.h"

static int initialized = 0;
static int n_restricted = 0;

static struct list_head *m_restrictions = NULL;

void print_restriction_info(struct restriction *r)
{
	pr_info("---");
	pr_info("restriction for pid %d\n", r->pid);
	pr_info("allowed pipes: %d\n", r->num_allowed_pipes);
	for (int i = 0; i < r->num_allowed_pipes; i++) {
		pr_info("\tpipe '%s'\n", r->pipe_restrictions[i].pipename);
	}
}

void debug_print_restrictions(void)
{
	struct list_head *restriction_iter;
	if (!m_restrictions) {
		pr_info("(debug) Module does not have any restrictions\n");
		return;
	}
	if (!initialized) {
		pr_info("(debug) Module not initialized\n");
		return;
	}
	list_for_each (restriction_iter, m_restrictions) {
		struct restriction *r =
			list_entry(restriction_iter, struct restriction, list);
		print_restriction_info(r);
	}
	pr_info("---");
}

struct restriction *get_restricted_process(pid_t process)
{
	struct list_head *restriction_iter;
	if (!m_restrictions) {
		return NULL;
	}
	if (!initialized) {
		return NULL;
	}
	list_for_each (restriction_iter, m_restrictions) {
		struct restriction *r =
			list_entry(restriction_iter, struct restriction, list);
		if (r->pid == process) {
			return r;
		}
	}

	return NULL;
}

int add_restriction(pid_t process)
{
	if (!initialized) {
		m_restrictions = vzalloc(sizeof(struct list_head));
		INIT_LIST_HEAD(m_restrictions);
		initialized = 1;
	}
	if (get_restricted_process(process) != NULL) {
		pr_err("Process is already restricted!\n");
		return 0;
	}

	struct restriction *r = kmalloc(sizeof(struct restriction), GFP_KERNEL);
	INIT_LIST_HEAD(&r->list);
	r->pid = process;
	list_add(&r->list, m_restrictions);
	n_restricted += 1;

	return 1;
}

int remove_restriction(pid_t process)
{
	if (!initialized) {
		pr_err("Module was not initialized\n");
		return 0;
	}

	struct restriction *r = get_restricted_process(process);
	if (get_restricted_process(process) == NULL) {
		pr_err("Process was not restricted!\n");
		return 0;
	}

	list_del(&r->list);
	kfree(r);

	return 1;
}

int restriction_has_pipe_allowed(pid_t process, char *pipename)
{
	if (!initialized) {
		pr_err("Module was not initialized\n");
		return 0;
	}

	struct restriction *r = get_restricted_process(process);
	if (r == NULL) {
		pr_err("Process was not restricted!\n");
		return 0;
	}

	for (int i = 0; i < r->num_allowed_pipes; i++) {
		if (strcmp(r->pipe_restrictions[i].pipename, pipename) == 0) {
			return 1;
		}
	}

	return 0;
}

int restriction_allow_pipe(pid_t process, char *pipename)
{
	if (!initialized) {
		pr_err("Module was not initialized\n");
		return 0;
	}

	if (strlen(pipename) >= PIPE_R_SIZE) {
		pr_err("Name of pipe is too long\n");
		return 0;
	}

	struct restriction *r = get_restricted_process(process);
	if (get_restricted_process(process) == NULL) {
		pr_err("Process was not restricted!\n");
		return 0;
	}

	if (r->num_allowed_pipes + 1 >= NUM_RESTRICTIONS) {
		pr_err("Process has maximum number of restricted pipes: %d\n",
		       r->num_allowed_pipes);
		return 0;
	}

	for (int i = 0; i < r->num_allowed_pipes; i++) {
		if (strcmp(r->pipe_restrictions[i].pipename, pipename) == 0) {
			pr_err("Pipe was already allowed!\n");
			return 0;
		}
	}

	strncpy(r->pipe_restrictions[r->num_allowed_pipes].pipename, pipename,
		PIPE_R_SIZE);
	r->num_allowed_pipes++;

	pr_info("Allowed pipe %s\n", pipename);
	return 1;
}
