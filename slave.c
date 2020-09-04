#define _POSIX_C_SOURCE 2
#define BUFFER_SIZE 1000
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

#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>



void processTask(const char * file);
void replaceChar(char * buffer, char oldChar, char newChar);

int main(int argc, char const *argv[])
{
    char buffer[BUFFER_SIZE];
    int len;


    if(setvbuf(stdout, NULL, _IONBF, 0) != 0)
        ERROR_CHECK(-1, "Slave - Setvbuf");

    for (size_t i = 1; i < argc; i++)
        processTask(argv[i]);

    while( (len = read(STDIN_FILENO,buffer,BUFFER_SIZE)) ){

        if (len == -1)
            ERROR_CHECK(len, "Slave - read");

        processTask(buffer);
        
    }

    return 0;
}

void processTask(const char * file){
   
    char cmd[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char * aux;
    sprintf(cmd, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", file);

    FILE *fp = popen(cmd, "r");

    if (fp == NULL)
        ERROR_CHECK(-1, "Slave - popen");

    int len = fread(buffer, sizeof(char), BUFFER_SIZE-1, fp);
    buffer[len] = 0;

    if (ferror(fp))
        ERROR_CHECK(-1, "Slave - fread");

    replaceChar(buffer,'\n','\t');

    printf("%s\n", buffer);
  // write(STDOUT_FILENO,buffer,len+1);

    ERROR_CHECK(pclose(fp), "Slave - pclose");
}

void replaceChar(char * buffer, char oldChar, char newChar){
    char * aux;
    while ((aux = strchr(buffer, oldChar)) != NULL)
        *aux = newChar;
}