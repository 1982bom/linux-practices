#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>

/**
 * kill processes which access descendants of given directory
 */
int is_number(const char *s) 
{
	size_t i, len = strlen(s);
	for (i = 0; i < len; i++) {
		if (!isdigit(s[i]))
			return 0;
	}
	return 1;
}

void kill_processes(const char *dir)
{
	DIR *par_pdir, *chl_pdir;
	struct dirent *par_pdent, *chl_pdent; 
	char path[PATH_MAX];

	par_pdir = opendir("/proc");
	if (par_pdir == NULL) {
		perror("opendir");
		return;
	}

	/* using children of  /proc/pid/fd, find out processes which accesses
	 * given directory and kill thoses
	 */
	while (par_pdent = readdir(par_pdir)) {
		if (isdigit(par_pdent->d_name[0]) && 
			((par_pdent->d_type == DT_DIR) || (par_pdent->d_type == DT_UNKNOWN))) {
			snprintf(path, sizeof(path), "/proc/%s/fd", par_pdent->d_name);
			chi_pdir = opendir(path);
			if (chl_pdir == NULL) {
				if (errno != EACCES || errno != ENOENT)
					perror(opendir);
				continue;
			}

			while (chl_pdent = readdir(chl_pdir)) {
			}		
		}
	}
	
}

int main(int argc, const char *argv[])
{
	kill_processes(argv[1]);
	return 0;
}
