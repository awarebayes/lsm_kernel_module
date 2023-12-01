/*
 * retstrict <pid>
 * unrestrict <pid>
 * allow pid network_socket <socket here>
 * allow pid pipe <pipe here>
*/

#include "linux/limits.h"
#include "linux/printk.h"
#include "linux/types.h"
#include <linux/kernel.h>
#include "storage.h"

#define BUFFER_SIZE 1024

int handle_allow_file(const char *command)
{
	char command_type[BUFFER_SIZE] = { 0 };
	char filename[PATH_MAX] = { 0 };
	char filetype[BUFFER_SIZE] = { 0 };
	int pid = 0;
	enum file_type ft = ALLOWED_FILE_REGULAR;

	if (sscanf(command, "%19s %d %1023s %20s", command_type, &pid, filename,
		   filetype) != 4) {
		pr_err("Invalid command format! Try: allow_file pid path/to/filename regular\n");
		pr_err("Allowed types are: regular, unix_socket, pipe\n");
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

int handle_allow_directory(const char *command)
{
	char command_type[BUFFER_SIZE] = { 0 };
	char filename[BUFFER_SIZE] = { 0 };
	char directory[PATH_MAX] = { 0 };
	int pid = 0;
	enum file_type ft = ALLOWED_FILE_REGULAR;

	if (sscanf(command, "%19s %d %1023s", command_type, &pid, directory) !=
	    3) {
		pr_err("Invalid command format! Try: allow_directory pid path/to/dir\n");
		return 0;
	}

	int dir_len = strlen(directory);
	if (directory[dir_len - 1] != '/') {
		pr_err("Directory must end with /, got %s\n", directory);
		return 0;
	}

	return restriction_allow_directory(pid, directory);
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
	} else if (strcmp(command_type, "allow_file") == 0) {
		if (!handle_allow_file(command)) {
			pr_err("handle_allow_file failed!\n");
			return 0;
		} else {
			pr_info("added allow file to pid %d\n", pid);
		}
	} else if (strcmp(command_type, "allow_directory") == 0) {
		if (!handle_allow_directory(command)) {
			pr_err("handle_allow_directory failed!\n");
			return 0;
		} else {
			pr_info("added allow directory to pid %d\n", pid);
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
