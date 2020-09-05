#define _GNU_SOURCE 2
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#define SEM_NAME "/IPC_Semaphore"
#define NAME "/IPC_Information"
#define SIZE 2000
#define MAX_BUFF 20
#define ERROR_CHECK(x, msg)                                                                              \
    do {                                                                                                 \
        int retval = (x);                                                                                \
        if (retval == -1) {                                                                              \
            fprintf(stderr, "Runtime error: %s returned %d at %s:%d\n", #x, retval, __FILE__, __LINE__); \
            perror(msg);                                                                                 \
            exit(-1);                                                                                    \
        }                                                                                                \
    } while (0)

int copyToShareMem(char *dest, const char *source);

int main(int argc, char const *argv[]) {
    
    int filesToRead;

    if (argc <= 1){
        char filesToReadBuff[MAX_BUFF];
        ERROR_CHECK(read(STDIN_FILENO, filesToReadBuff, MAX_BUFF), "View - read stdin");
        filesToRead = atoi(filesToReadBuff);
        printf("files to Read %d\n",filesToRead);
    }else{
        filesToRead = atoi(argv[1]);
    }
    
    struct stat myStat;
    int shm_fd;
    char * ptr;
    char * shm_base;
    sem_t * sem_id = sem_open(SEM_NAME, O_CREAT,0666,0);
    if(sem_id == SEM_FAILED){
        ERROR_CHECK(-1,"Opening semaphore failed");
    }

    ERROR_CHECK((shm_fd = shm_open(NAME, O_RDONLY, 0)), "View - shm_open");

    ERROR_CHECK(fstat(shm_fd, &myStat), "View - fstat");

    shm_base = mmap(NULL, myStat.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    ptr = shm_base;
    if (ptr == MAP_FAILED)
        ERROR_CHECK(-1, "View - mmap");

    int outputsRead = 0;
    char buffer[SIZE];
    while(outputsRead < filesToRead){

        ERROR_CHECK(sem_wait(sem_id), "View - sem wait");
        ptr += copyToShareMem(buffer,ptr);
        outputsRead++;
        printf("%s\n",buffer);
    }

    ERROR_CHECK(sem_close(sem_id),"Closing semaphore errror");
   
    int state = sem_unlink(SEM_NAME);
    if(state != 0 && state != ENOENT){
        ERROR_CHECK(-1,"Unlink semaphore error");
    }
    
    ERROR_CHECK(munmap(shm_base, myStat.st_size), "View - munmap");

    ERROR_CHECK(close(shm_fd), "View - close");

    ERROR_CHECK(shm_unlink(NAME), "View - shm_unlink");

    return 0;
}


int copyToShareMem(char *dest, const char *source) {
    int i;
    for ( i = 0; source[i] != 0; i++) {
        dest[i] = source[i];
    }

    dest[i] = 0;

    return i + 1;
}
