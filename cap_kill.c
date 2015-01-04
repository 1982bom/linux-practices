#include <sys/types.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <linux/capability.h>

/**
 * CAP_KILL is granted and kill task with given pid
 */

void show_capset()
{
	FILE *filp;
	char line[256];

	filp = fopen("/proc/self/status", "r");
	if (filp == NULL) {
		perror("fopen");
		return;
	}
	
	while (fgets(line, sizeof(line), filp)) {
		if (strncmp(line, "Cap", 3) == 0)
			printf("%s", line);
	}
	
	fclose(filp);
}

void add_cap(cap_user_data_t data, __u32 cap)
{
	data[CAP_TO_INDEX(cap)].permitted |= CAP_TO_MASK(cap);
	data[CAP_TO_INDEX(cap)].inheritable |= CAP_TO_MASK(cap);
	data[CAP_TO_INDEX(cap)].effective |= CAP_TO_MASK(cap);
}

int main(int argc, const char *argv[])
{
	cap_user_header_t head;
	cap_user_data_t data;
	pid_t pid;

	pid = atoi(argv[1]);
	
	head = calloc(1, sizeof(*head));
	head->version = _LINUX_CAPABILITY_VERSION;
	head->pid = 0;

	// set CAP_KILL and CAPSETUID
	data = calloc(_LINUX_CAPABILITY_U32S, sizeof(*data));
	add_cap(data, CAP_KILL);
	add_cap(data, CAP_SETUID);
	if (capset(head, data) < 0) {
		perror("capset");
		return 1;
	}

	// for capabilities not to be cleared when uid is changed
	if (prctl(PR_SET_KEEPCAPS, 1) != 0) {
		perror("prctl");
		return 1;
	}

	// uid to hyeoncheol
	if (setuid(1000) != 0) {
		perror("setuid");
		return 1;
	}

	// set CAP_KILL to effective capability set
	memset(data, 0, _LINUX_CAPABILITY_U32S * sizeof(*data));
	add_cap(data, CAP_KILL);
	if (capset(head, data) < 0) {
		perror("capset");
		return 1;
	}

	kill(-pid, SIGKILL);
	return 0;
}
