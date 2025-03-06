// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    // printf("%s ",d->d_name);
    for (int i=0; i<4; i++) {
        if (strncmp(d->d_name, ttyusb_prefixes[i], strlen(ttyusb_prefixes[i])) == 0) {
            return 1;
        }
    }

    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    // unimplemented();

    char *buffer = calloc(100, 1);
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, alphasort);
    if (n == 0) {
        panic("No name occurs!\n");
    }
    else if (n > 1) {
        panic("Too many names occur!\n");
    }

    buffer = strdupf(namelist[0]->d_name);
    
    for (int i=0; i<n; i++) {
        free(namelist[i]);
    }
    
    memcpy(buffer + strlen("/dev/"), buffer, 100);
    memcpy(buffer, "/dev/", strlen("/dev/"));

    return buffer;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    // unimplemented();

    char *buffer = calloc(100, 1);
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, alphasort);
    if (n == 0) {
        panic("No name occurs!\n");
    }
    // else if (n > 1) {
    //     panic("Too many names occur!\n");
    // }

    struct stat fileStat;
    char *newest_file = NULL;
    time_t newest_time;
    const char *prefix = "/dev/";
    if (n == 1) {
        newest_file = namelist[0]->d_name;
    }
    else {
        for (int i=0; i<n; i++) {
            char tmp_buffer[100];
            snprintf(tmp_buffer, sizeof(tmp_buffer), "%s%s", prefix, namelist[i]->d_name);
            if (stat(tmp_buffer, &fileStat) == -1) {
                panic("file error: %s", tmp_buffer);
                continue;
            }
            if (newest_file == NULL || fileStat.st_mtime > newest_time) {
                newest_time = fileStat.st_mtime;
                newest_file = namelist[i]->d_name;
            }
        }
    }
    buffer = strdupf(newest_file);
    
    for (int i=0; i<n; i++) {
        free(namelist[i]);
    }

    memcpy(buffer + strlen("/dev/"), buffer, 100);
    memcpy(buffer, "/dev/", strlen("/dev/"));
    

    return buffer;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    // unimplemented();
    char *buffer = calloc(100, 1);
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, alphasort);
    if (n == 0) {
        panic("No name occurs!\n");
    }
    // else if (n > 1) {
    //     panic("Too many names occur!\n");
    // }

    struct stat fileStat;
    char *oldest_file = NULL;
    time_t oldest_time;
    const char *prefix = "/dev/";
    if (n == 1) {
        oldest_file = namelist[0]->d_name;
    }
    else {
        for (int i=0; i<n; i++) {
            char tmp_buffer[100];
            snprintf(tmp_buffer, sizeof(tmp_buffer), "%s%s", prefix, namelist[i]->d_name);
            if (stat(tmp_buffer, &fileStat) == -1) {
                panic("file error");
                continue;
            }
            if (oldest_file == NULL || fileStat.st_mtime < oldest_time) {
                oldest_time = fileStat.st_mtime;
                oldest_file = namelist[i]->d_name;
            }
        }
    }
    buffer = strdupf(oldest_file);
    
    for (int i=0; i<n; i++) {
        free(namelist[i]);
    }

    memcpy(buffer + strlen("/dev/"), buffer, 100);
    memcpy(buffer, "/dev/", strlen("/dev/"));


    return buffer;
}
