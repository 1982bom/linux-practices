#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

/**
 * example to run single instance of program
 * using flock, prevent others from running program while one runs 
 */

int main() 
{
	pid_t pid;
	int fd, lfd, pfd[2];

	if (pipe(pfd) < 0) {
		perror("pfd");
		return 1;
	}

	printf("parent[%d]: tries to locks flock\n", getpid());
	lfd = open(".lock_for_lock.flock", O_WRONLY | O_CREAT, 0600);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	printf("parent[%d]: tries to locks file\n", getpid());
	fd = open(".lock_for_single_process.flock", O_WRONLY | O_CREAT, 0600);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	// lock to prevent race condition for fd
	if (lockf(lfd, F_LOCK, 0) != 0) {
		perror("lockf");
		return 1;
	}

	// lock to run single instance of program
	if (lockf(fd, F_LOCK, 0) != 0) {
		perror("lockf");
		return 1;
	}

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	} else if (pid == 0) {
		printf("child[%d]: tries locks file\n", getpid());
		if (lockf(fd, F_LOCK, 0) != 0) {
			perror("child: lock");
			return 1;
		}
		printf("child[%d]: let parent know that child lock file\n", getpid());
		close(pfd[0]);
		close(pfd[1]);

		printf("child: falls asleep\n");
		sleep(20);
		printf("child: unlock file\n");
		if (lockf(fd, F_ULOCK, 0) != 0) {
			perror("child: unlock");
			return 1;
		}
		close(fd);
		close(lfd);
		return 0;
	} else {
		int val;

		printf("parent[%d]: falls asleep\n", getpid());
		sleep(10);

		printf("parent[%d]: unlocks file\n", getpid());
		if (lockf(fd, F_ULOCK, 0) != 0) {
			perror("parent: unlock");
			return 1;
		}

		printf("parent[%d]: waits for child to lock file\n", getpid());
		close(pfd[1]);
		read(pfd[0], &val, sizeof(val)); 
		close(pfd[0]);

		printf("parent[%d]: waits for child to exit\n", getpid());
		wait(NULL);
		close(fd);
		close(lfd);
		return 0;
	}
}
