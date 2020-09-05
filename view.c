#define _GNU_SOURCE 2
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#define NAME "/myInfo"
#define SIZE 2000
#define ERROR_CHECK(x, msg)                                                                              \
    do                                                                                                   \
    {                                                                                                    \
        int retval = (x);                                                                                \
        if (retval == -1)                                                                                \
        {                                                                                                \
            fprintf(stderr, "Runtime error: %s returned %d at %s:%d\n", #x, retval, __FILE__, __LINE__); \
            perror(msg);                                                                                 \
            exit(-1);                                                                                    \
        }                                                                                                \
    } while (0)


int main(int argc, char const *argv[]){
    
    // int files = atoi(argv[1]);


    struct stat myStat;
    int shm_fd;

    ERROR_CHECK((shm_fd = shm_open(NAME, O_RDONLY,0)), "View - shm_open");

    ERROR_CHECK(fstat(shm_fd,&myStat), "View - fstat");

    char * ptr = mmap(NULL, myStat.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if(ptr==MAP_FAILED)
        ERROR_CHECK(-1, "View - mmap");

    ////////////////////////////////
    printf("%s\n",ptr);
    ////////////////////////////////

    ERROR_CHECK(munmap(ptr, myStat.st_size), "View - munmap");

    ERROR_CHECK(close(shm_fd), "View - close");

    ERROR_CHECK(shm_unlink(NAME), "View - shm_unlink");
    
    return 0;
}
