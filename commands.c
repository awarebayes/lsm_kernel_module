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

int handle_allow_pipe(const char *command)
{
	char command_type[BUFFER_SIZE] = { 0 };
	char pipename[BUFFER_SIZE] = { 0 };
	int pid = 0;

	if (sscanf(command, "%19s %d %20s", command_type, &pid, pipename) !=
	    3) {
		pr_err("Invalid command format! Try: allow_pipe pid path/to/pipe_name.pipe \n");
		return 0;
	}

	return restriction_allow_pipe(pid, pipename);
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
		if (!handle_allow_pipe(command)) {
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
