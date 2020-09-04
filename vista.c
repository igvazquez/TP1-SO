// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <sys/mman.h>
#include <sys/stat.h>       
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


#define NAME "/myInfo"
#define SIZE 2000

int main(int argc, char const *argv[]){
    
    struct stat myStat;

    int fd= shm_open("/myInfo", O_RDONLY,0); // cero porque solo tiene sentido en el momento cuando se crea los permisos
    if (fd==-1)
    {
        //ERROR
        return -1;
    }
    if (fstat(fd,&myStat)==-1){
        //ERROR
        return -1;
    }
    char * ptr = mmap(NULL,myStat.st_size,PROT_READ,MAP_SHARED,fd,0);
    if(ptr==MAP_FAILED){
        //EXIT
        return -1;
    }

    printf("%s\n",ptr);

    if(munmap(ptr,myStat.st_size)==-1){
        //ERROR
        return -1;
    }

    if(shm_unlink(NAME)==-1){
        //ERROR
        return -1;
    }
    
    close (fd);
    
    return 0;
}
