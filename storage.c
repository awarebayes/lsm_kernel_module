#include "storage.h"
#include "linux/list.h"
#include "linux/printk.h"
#include "linux/slab.h"

static int initialized = 0;
static int n_restricted = 0;
static struct list_head *m_restrictions = NULL;
static char result_buffer[RETURN_BUFFER_SIZE];

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
	pr_info("allowed ips: %d\n", r->num_allowed_ips);
	for (int i = 0; i < r->num_allowed_ips; i++) {
		pr_info("\tipv4 '%s'\n", r->ip_restrictions[i].ip_str);
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

void print_csv_restriction_info(pid_t process)
{
	if (!m_restrictions) {
		pr_info("(debug) Module does not have any restrictions\n");
		return;
	}
	if (!initialized) {
		pr_info("(debug) Module not initialized\n");
		return;
	}
	struct restriction *r = get_restricted_process(process);
	if (r == NULL) {
		pr_info("requested non existent pid to info about for seqfile");
	}
	char buffer[4096] = { 0 };
	char *buffer_ptr = buffer;
	char *buffer_end = buffer + 4096;

	buffer_ptr += snprintf(buffer_ptr, buffer_end - buffer_ptr,
			       "restriction start\n");
	buffer_ptr += snprintf(buffer_ptr, buffer_end - buffer_ptr, "pid %d\n",
			       r->pid);
	if (r->dir_restricted) {
		buffer_ptr += snprintf(buffer_ptr, buffer_end - buffer_ptr,
				       "dir %s\n", r->working_dir);
	}
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
		buffer_ptr += snprintf(buffer_ptr, buffer_end - buffer_ptr,
				       "file %s %s\n", ft_buf,
				       r->file_restrictions[i].filename);
	}
	for (int i = 0; i < r->num_allowed_ips; i++) {
		buffer_ptr +=
			snprintf(buffer_ptr, buffer_end - buffer_ptr,
				 "ipv4 %s\n", r->ip_restrictions[i].ip_str);
	}
	buffer_ptr += snprintf(buffer_ptr, buffer_end - buffer_ptr,
			       "restriction end\n");
	return_buffer_set(buffer);
}

void print_csv_restricted_pids(void)
{
	char buffer[4096] = { 0 };
	char *buffer_ptr = buffer;
	char *buffer_end = buffer + 4096;

	buffer_ptr +=
		snprintf(buffer_ptr, buffer_end - buffer_ptr, "pids start\n");

	if (!m_restrictions || !initialized) {
		buffer_ptr += snprintf(buffer_ptr, buffer_end - buffer_ptr,
				       "pids end\n");
		return_buffer_set(buffer);

		return;
	}
	struct list_head *restriction_iter;

	list_for_each (restriction_iter, m_restrictions) {
		struct restriction *r =
			list_entry(restriction_iter, struct restriction, list);
		buffer_ptr += snprintf(buffer_ptr, buffer_end - buffer_ptr,
				       "%d\n", r->pid);
	}

	buffer_ptr +=
		snprintf(buffer_ptr, buffer_end - buffer_ptr, "pids end\n");
	return_buffer_set(buffer);
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

int restriction_has_ip_allowed(pid_t process, char *ip_str)
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

	if (r->num_allowed_ips == 0) {
		pr_info("Process has no allowed ips...\n");
	}

	for (int i = 0; i < r->num_allowed_ips; i++) {
		if (strcmp(r->ip_restrictions[i].ip_str, ip_str) == 0) {
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
	if (r == NULL) {
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

int restriction_allow_ip(pid_t process, char *ip_str)
{
	if (!initialized) {
		pr_err("Module was not initialized\n");
		return 0;
	}

	if (strlen(ip_str) >= IP_SIZE) {
		pr_err("Ip string is too long\n");
		return 0;
	}

	struct restriction *r = get_restricted_process(process);
	if (r == NULL) {
		pr_err("Process was not restricted!\n");
		return 0;
	}

	if (r->dir_restricted != 0) {
		pr_err("Pid %d has already restricted to work within directory %s\n",
		       process, r->working_dir);
		return 0;
	}

	if (r->num_allowed_ips + 1 >= NUM_RESTRICTIONS) {
		pr_err("Process has maximum number of restricted ips: %d\n",
		       r->num_allowed_ips);
		return 0;
	}

	for (int i = 0; i < r->num_allowed_files; i++) {
		if (strcmp(r->ip_restrictions[i].ip_str, ip_str) == 0) {
			pr_err("IP was already allowed!\n");
			return 0;
		}
	}

	strncpy(r->ip_restrictions[r->num_allowed_ips].ip_str, ip_str, IP_SIZE);
	r->num_allowed_ips++;

	return 1;
}

int return_buffer_set(const char *str)
{
	int n_chars = strlen(str);
	if (n_chars > RETURN_BUFFER_SIZE) {
		return 0;
	}
	memset(result_buffer, 0, n_chars);
	memcpy(result_buffer, str, n_chars);
	return 1;
}

const char *return_buffer_get(void)
{
	return result_buffer;
}
