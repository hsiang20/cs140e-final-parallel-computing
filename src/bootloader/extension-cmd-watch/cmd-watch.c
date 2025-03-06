// simple program to run a command when any file that is "interesting" in a directory
// changes.
// e.g., 
//      cmd-watch make
// will run make at each change.
//
// This should use the scandir similar to how you did `find_ttyusb`
//
// key part will be implementing two small helper functions (useful-examples/ will be 
// helpful):
//  - static int pid_clean_exit(int pid);
//  - static void run(char *argv[]);
//
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>

// scan the files in "./" (you can extend this) for those
// that match the suffixes in <suffixes> and check  if any
// have changed since the last time.
int check_activity(void) {
    char *suffixes[] = { ".c", ".h", ".S", "file", 0 };
    int num_of_suffixes = 4;
    const char *dirname = ".";
    int changed_p = 0;

    static time_t last_mtime;   // store last modification time.

    // unimplemented();
    struct dirent *entry;
    struct stat st;

    DIR *dir = opendir(dirname);

    while ((entry = readdir(dir)) != NULL) {
        size_t len = strlen(entry->d_name);
        for (size_t i = 0; i < num_of_suffixes; i++) {
            size_t suf_len = strlen(suffixes[i]);
            if (len > suf_len && strcmp(entry->d_name + len - suf_len, suffixes[i]) == 0) {
                if (stat(entry->d_name, &st) == 0) {
                    if (!last_mtime) {
                        last_mtime = st.st_mtime;
                        changed_p = 1;
                    }
                    else if (st.st_mtime > last_mtime) {
                        last_mtime = st.st_mtime;
                        changed_p = 1;
                    }
                }
            }
        }
    }

    closedir(dir);

    // return 1 if anything that matched <suffixes>
    return changed_p;
}

// synchronously wait for <pid> to exit.  returns 1 if it exited
// cleanly (via exit(0)), 0 otherwise.
static int pid_clean_exit(int pid) {
    unimplemented();
}

// simple helper to print null terminated vector of strings.
static void print_argv(char *argv[]) {
    assert(argv[0]);

	fprintf(stderr, "about to exec =<%s ", argv[0]);
	for(int i =1; argv[i]; i++)
		fprintf(stderr, " %s", argv[i]);
	fprintf(stderr, ">\n");
}


// fork/exec <argv> and wait for the result: print an error
// and exit if the kid crashed or exited with an error (a non-zero
// exit code).
static void run(char *argv[]) {
    // unimplemented();
    pid_t pid = fork();
    if (pid == 0) { // Child process
        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    } else if (pid > 0) { // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command exited with error code %d\n", WEXITSTATUS(status));
        }
    } else {
        perror("fork");
        exit(1);
    }

}

int main(int argc, char *argv[]) {
    if(argc < 2)
        die("cmd-watch: not enough arguments\n");
        
    // unimplemented();
    int changed_p;

    while (1) {
        changed_p = check_activity();

        if (changed_p) {
            run(&argv[1]);
        }

        usleep(500000);

    }


    return 0;
}
