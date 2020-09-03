#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 2000
#define FILE_SIZE 50
#define ERROR_CHECK(x, msg) do { \
  int retval = (x); \
  if (retval == -1) { \
    fprintf(stderr, "Runtime error: %s returned %d at %s:%d\n", #x, retval, __FILE__, __LINE__); \
    perror(msg); \
    exit(-1); \
  } \
} while (0)

int main(int argc, char const *argv[])
{
    char buffer[BUFFER_SIZE];
    char file[FILE_SIZE];
    int len;
    int fd;

    printf("slave %d running with %d files\n", getpid(),argc-1);

    for (size_t i = 1; i < argc; i++){
        sprintf(buffer, "tail %s", argv[i]);
        // printf("%d\t%s\t\n", getpid(), argv[i]);
        printf("pre popen\n");
        FILE * fp = popen(buffer, "r");
        if (fp == NULL)
            ERROR_CHECK(-1, "Slave - popen");

        while (fgets(buffer, BUFFER_SIZE, fp) != NULL)
            printf("%s", buffer);
        
        printf("post popen\n");

        ERROR_CHECK(pclose(fp), "Slave - pclose");
    }

    // while( (len = read(STDIN_FILENO,buffer,BUFFER_SIZE) ) > 0){

    //     write(STDOUT_FILENO,buffer,BUFFER_SIZE);


/*
        for (size_t i = 0,j=0; buffer[i] != 0 ; i++)
        {
            if(buffer[i]!= '\n'){
                file[j++]=buffer[i];
            }
            else{
                file[j]=0;
                j=0;
                if(fd=open(file,O_RDONLY)==-1)
                    //ERROR 
                
            }
  */      
    // }

    return 0;
    }



