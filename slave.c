#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 300
#define FILE_SIZE 50
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

void processTask(char * file);
//grep -o -e "Number of.*[0-9]\+" -e "CPU time.*" -e ".*SATISFIABLE"

int main(int argc, char const *argv[])
{
    char cmd[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char file[FILE_SIZE];
    int len, fd;

    if(setvbuf(stdout, NULL, _IONBF, 0) != 0)
        ERROR_CHECK(-1, "Slave - Setvbuf");

    // printf("slave %d running with %d files\n", getpid(), argc - 1);

    for (size_t i = 1; i < argc; i++)
        processTask(argv[i]);

    while(len = read(STDIN_FILENO,buffer,BUFFER_SIZE)){
       
        if (len == -1)
            ERROR_CHECK(len, "Slave - read");

        processTask(buffer);
        
    }

    return 0;
}

void processTask(char * file){
   
    char cmd[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    sprintf(cmd, "tail %s", file);

    FILE *fp = popen(cmd, "r");

    if (fp == NULL)
        ERROR_CHECK(-1, "Slave - popen");

    int len = fread(buffer, sizeof(char), BUFFER_SIZE-1, fp);
    buffer[len] = 0;

    if (ferror(fp))
        ERROR_CHECK(-1, "Slave - fread");

    printf("%s\n", buffer);

    ERROR_CHECK(pclose(fp), "Slave - pclose");
}
