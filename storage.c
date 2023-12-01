#include "storage.h"
#include "linux/list.h"
#include "linux/slab.h"

static int initialized = 0;
static int n_restricted = 0;

static struct list_head *m_restrictions = NULL;

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
		pr_info("List item pid %d\n", r->pid);
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

	struct restriction *r = vzalloc(sizeof(struct restriction));
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
