#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    // How: 
    //    - use stat() to get the size of the file.
    //    - round up to a multiple of 4.
    //    - allocate a buffer
    //    - zero pads to a multiple of 4.
    //    - read entire file into buffer (read_exact())
    //    - fclose() the file descriptor
    //    - make sure any padding bytes have zeros.
    //    - return it.   
    // unimplemented();
    struct stat fileStat;

    if (stat(name, &fileStat) == 0) {
        *size = (unsigned)fileStat.st_size;
    }
    else {
        // printk("file error!\n");
        panic("file error\n");
    }

    char *buffer = calloc(*size + 4, 1);
    if (!buffer) {
        panic("buffer error\n");
    }
    
    int fd = open(name, O_RDONLY);

    if (*size != 0) {
        int read_result = read_exact(fd, buffer, *size);
    }
    close(fd);
    return buffer;
}
