#include "storage.h"
#include "linux/list.h"
#include "linux/printk.h"
#include "linux/slab.h"

static int initialized = 0;
static int n_restricted = 0;

static struct list_head *m_restrictions = NULL;

#define BUF_SIZE 256

void print_restriction_info(struct restriction *r)
{
	pr_info("---");
	pr_info("restriction for pid %d\n", r->pid);
	if (r->dir_restricted) {
		pr_info("restricted to directory: %s\n", r->working_dir);
	} else {
		pr_info("restricted to no directory, can only open allowed files\n");
	}
	pr_info("allowed files: %d\n", r->num_allowed_files);
	for (int i = 0; i < r->num_allowed_files; i++) {
		char ft_buf[BUF_SIZE] = { 0 };
		switch (r->file_restrictions[i].type) {
		case ALLOWED_FILE_REGULAR:
			sprintf(ft_buf, "regular");
			break;
		case ALLOWED_FILE_PIPE:
			sprintf(ft_buf, "file_pipe");
			break;
		case ALLOWED_FILE_UNIX_SOCKET:
			sprintf(ft_buf, "unix_socket");
			break;
		default:
			sprintf(ft_buf, "unknown");
			break;
		}
		pr_info("\tfile %s '%s'\n", ft_buf,
			r->file_restrictions[i].filename);
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
	memset(r, 0, sizeof(struct restriction));
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
	n_restricted -= 1;

	return 1;
}

int restriction_has_file_allowed(pid_t process, char *filename,
				 enum file_type ft)
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

	for (int i = 0; i < r->num_allowed_files; i++) {
		if (strcmp(r->file_restrictions[i].filename, filename) == 0 &&
		    ft == r->file_restrictions[i].type) {
			return 1;
		}
	}

	return 0;
}

int restriction_allow_file(pid_t process, char *filename, enum file_type ft)
{
	if (!initialized) {
		pr_err("Module was not initialized\n");
		return 0;
	}

	if (strlen(filename) >= FILE_R_SIZE) {
		pr_err("Name of file is too long\n");
		return 0;
	}

	struct restriction *r = get_restricted_process(process);
	if (get_restricted_process(process) == NULL) {
		pr_err("Process was not restricted!\n");
		return 0;
	}

	if (r->num_allowed_files + 1 >= NUM_RESTRICTIONS) {
		pr_err("Process has maximum number of restricted filename: %d\n",
		       r->num_allowed_files);
		return 0;
	}

	for (int i = 0; i < r->num_allowed_files; i++) {
		if (strcmp(r->file_restrictions[i].filename, filename) == 0) {
			pr_err("File was already allowed!\n");
			return 0;
		}
	}

	strncpy(r->file_restrictions[r->num_allowed_files].filename, filename,
		FILE_R_SIZE);
	r->file_restrictions[r->num_allowed_files].type = ft;
	r->num_allowed_files++;

	pr_info("Allowed file %s\n", filename);
	return 1;
}

int restriction_allow_directory(pid_t process, char *dirname)
{
	if (!initialized) {
		pr_err("Module was not initialized\n");
		return 0;
	}

	if (strlen(dirname) >= PATH_MAX) {
		pr_err("Name of directory is too long\n");
		return 0;
	}

	struct restriction *r = get_restricted_process(process);
	if (get_restricted_process(process) == NULL) {
		pr_err("Process was not restricted!\n");
		return 0;
	}

	if (r->dir_restricted != 0) {
		pr_err("Pid %d has already restricted to work within directory %s\n",
		       process, r->working_dir);
		return 0;
	}

	strncpy(r->working_dir, dirname, PATH_MAX);
	r->dir_restricted = 1;

	pr_info("Restricted to directory %s\n", dirname);
	return 1;
}
