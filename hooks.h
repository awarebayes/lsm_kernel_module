#include <linux/fs.h>
#include <linux/net.h>

int my_security_file_open(struct file *file);
int my_security_socket_connect(struct socket *sock, struct sockaddr *addr,
			       int addrlen);
