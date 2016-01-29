#include <errno.h>
#include <CHANTILLY_BP.h>
#include "../common/CHAN/daemon_defs.h"
#include "../common/CHAN/daemon_utils.h"

#define _THIS_APP_ID 	0xFE

pid_t create_process(void)
{
    pid_t pid;
    do {
		pid = fork();
    } while ((pid == -1) && (errno == EAGAIN));
    return pid;
}

int main (int argc, char *argv[ ])
{
	CHAN_setup("chalt",1);
	CHAN_command(7,6);
	char *arg[] = {"shutdown", "-h","now",NULL};
	pid_t pid;
	pid=create_process();
	switch (pid) {
		case -1:
			perror("fork");
			return EXIT_FAILURE;
		break;
		
		case 0 :// child_process
			if (execv("/sbin/shutdown", arg) == -1) {
				perror("execv");
				exit(EXIT_FAILURE);
			}
		break;
		default :// father_process
		break;
	}
	return 0;
}
