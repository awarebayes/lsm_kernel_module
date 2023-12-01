/*
 * retstrict <pid>
 * allow pid network_socket <socket here>
 * allow pid pipe <pipe here>
*/

#include "linux/printk.h"
#include "linux/types.h"
#include <linux/kernel.h>
#include "storage.h"

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
	} else if (strcmp(command_type, "allow") == 0) {
		if (!remove_restriction(pid)) {
			pr_err("remove_restriction failed!\n");
			return 0;
		} else {
			pr_info("unrestricted pid %d\n", pid);
		}
	} else {
		pr_err("unknown command issued %s!\n", command_type);
		return 0;
	}
	return 1;
}
