/*
 * retstrict <pid>
 * unrestrict <pid>
 * allow pid network_socket <socket here>
 * allow pid pipe <pipe here>
*/

#include "linux/printk.h"
#include "linux/types.h"
#include <linux/kernel.h>
#include "storage.h"

#define BUFFER_SIZE 256

int handle_allow_file(const char *command)
{
	char command_type[BUFFER_SIZE] = { 0 };
	char filename[BUFFER_SIZE] = { 0 };
	char filetype[BUFFER_SIZE] = { 0 };
	int pid = 0;
	enum file_type ft = ALLOWED_FILE_REGULAR;

	if (sscanf(command, "%19s %d %20s %20s", command_type, &pid, filename,
		   filetype) != 4) {
		pr_err("Invalid command format! Try: allow_file pid path/to/filename regular\n");
		return 0;
	}

	if (strcmp(filetype, "regular") == 0) {
		ft = ALLOWED_FILE_REGULAR;
	} else if (strcmp(filetype, "unix_socket") == 0) {
		ft = ALLOWED_FILE_UNIX_SOCKET;
	} else if (strcmp(filetype, "pipe") == 0) {
		ft = ALLOWED_FILE_PIPE;
	} else {
		pr_err("Allowed types are: regular, unix_socket, pipe but I got %s\n",
		       filetype);
		return 0;
	}

	return restriction_allow_file(pid, filename, ft);
}

int parse_command(const char *command)
{
	char command_type[BUFFER_SIZE] = { 0 };
	int pid = 0;
	// Use sscanf to check the number of successfully parsed items
	if (sscanf(command, "%19s %d", command_type, &pid) != 2) {
		pr_err("Invalid command format!\n");
		return 0;
	}

	if (strcmp(command_type, "restrict") == 0) {
		if (!add_restriction(pid)) {
			pr_err("add_restriction failed!\n");
			return 0;
		} else {
			pr_info("Restricted pid %d\n", pid);
		}
	} else if (strcmp(command_type, "unrestrict") == 0) {
		if (!remove_restriction(pid)) {
			pr_err("remove_restriction failed!\n");
			return 0;
		} else {
			pr_info("unrestricted pid %d\n", pid);
		}
	} else if (strcmp(command_type, "allow_pipe") == 0) {
		if (!handle_allow_file(command)) {
			pr_err("handle_allow failed!\n");
			return 0;
		} else {
			pr_info("added allow to pid %d\n", pid);
		}
	} else if (strcmp(command_type, "debug") == 0) {
		debug_print_restrictions();
	} else {
		pr_err("unknown command issued %s! I can handle: restrict, unrestrict, debug, allow_pipe, allow_socket\n",
		       command_type);
		return 0;
	}
	return 1;
}
