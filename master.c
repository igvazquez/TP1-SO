#define SLAVE "./slave.out"
#define MAX_SLAVES 5
#define PIPE_READ 0
#define PIPE_WRITE 1
#define MAX_BUFFER_SIZE 200
#define INITIAL_TASKS 2
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

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

typedef struct slave_t
{
    pid_t pid;
    int fdIn;
    int fdOut;
    int pending;
} slave_t;

int createChildren(slave_t children[], char *tasksRemaining[], int *filesRead, int filesLeft);
int assignTask(char *tasksRemaining[], int *filesRead, int filesLeft, int fd);
int worksProcessed(char *buffer);
void replaceChar(char * buffer, char oldChar, char newChar);

int main(int argc, char const *argv[])
{
    if (argc == 1)
    {
        return 0;
    }

    int filesLeft = argc - 1;
    int filesRead = 0, fdsAvailable = 0, tasksDone = 0,charsRead = 0;
    char **tasksRemaining = argv + 1;
    slave_t children[MAX_SLAVES];
    char * token;
    char * delim = "\n";

    if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
        ERROR_CHECK(-1, "Master - Setvbuf");

    int childrenCreated = createChildren(children, tasksRemaining, &filesRead, filesLeft);

    fd_set fdSet;
    char buffer[250];

    while (tasksDone < filesLeft)
    {

        FD_ZERO(&fdSet);

        for (size_t i = 0; i < childrenCreated; i++)
        {
            FD_SET(children[i].fdOut, &fdSet);
        }
        ERROR_CHECK(fdsAvailable = select(children[childrenCreated - 1].fdOut + 1, &fdSet, NULL, NULL, NULL), "Master - select");
        for (int i = 0; i < childrenCreated && fdsAvailable > 0; i++){

            if (FD_ISSET(children[i].fdOut, &fdSet)){

                ERROR_CHECK(charsRead = read(children[i].fdOut, buffer, MAX_BUFFER_SIZE - 1), "Master - read"); // Leer los outputs del slave
                buffer[charsRead] = 0;
                
                if(charsRead){
                    
                    token = strtok(buffer,delim);
                    while(token != NULL){
                        replaceChar(token,'\t','\n');
//                        printf("Output hijo %d: %s\n", i, token);
                        printf("%s\n",token);
  
                        children[i].pending--;
                        tasksDone++;
  //                      printf("tasksDone: %d\n", tasksDone);
                        token = strtok(NULL,delim);
                    }

                    if(children[i].pending == 0){
                        children[i].pending += assignTask(tasksRemaining, &filesRead, filesLeft, children[i].fdIn);
                    }
                }
                fdsAvailable--;
            }
        }
    }

    return 0;
}

int createChildren(slave_t children[], char *tasksRemaining[], int *filesRead, int filesLeft)
{

    int i, id, pendings = 1;
    char *initArgsArray[INITIAL_TASKS + 2];

    if (filesLeft > MAX_SLAVES * INITIAL_TASKS)
    {
        pendings = INITIAL_TASKS;
    }

    for (i = 0; i < MAX_SLAVES && *filesRead < filesLeft; i++)
    {

        int pipeMtoS[2];
        int pipeStoM[2];

        if (pipe(pipeMtoS) == -1)
            return 1; //TODO: error managing

        if (pipe(pipeStoM) == -1)
            return 1; //TODO: error managing

        if ((id = fork()) == -1)
        {
            return 2; //TODO: error managing
        }
        else if (id == 0)
        {
            ERROR_CHECK(close(pipeMtoS[PIPE_WRITE]), "Master - close pipeMtoS[WRITE]");
            ERROR_CHECK(close(pipeStoM[PIPE_READ]), "Master - close pipeStoM[READ]");
            ERROR_CHECK(dup2(pipeMtoS[PIPE_READ], STDIN_FILENO), "pipeMtoS dup2");
            ERROR_CHECK(dup2(pipeStoM[PIPE_WRITE], STDOUT_FILENO), "pipeStoM dup2");
            ERROR_CHECK(close(pipeMtoS[PIPE_READ]), "Master - close pipeMtoS[READ]");
            ERROR_CHECK(close(pipeStoM[PIPE_WRITE]), "Master - close pipeStoM[WRITE]");
            initArgsArray[0] = SLAVE;
            for (size_t j = 1; j <= pendings; j++)
                initArgsArray[j] = tasksRemaining[(*filesRead)++];
            initArgsArray[pendings + 1] = NULL;
            ERROR_CHECK(execv(SLAVE, initArgsArray), "Master - execv"); // Solo se ejecuta error_check si execv falla
        }
        else
        {
            *filesRead += pendings;
            children[i].pid = id;
            children[i].fdIn = pipeMtoS[PIPE_WRITE];
            children[i].fdOut = pipeStoM[PIPE_READ];
            children[i].pending = pendings;
            ERROR_CHECK(close(pipeMtoS[PIPE_READ]), "Master - close pipeMtoS[READ]");
            ERROR_CHECK(close(pipeStoM[PIPE_WRITE]), "Master - pipeStoM[WRITE]");
        }
    }
    return i;
}

int assignTask(char *tasksRemaining[], int *filesRead, int filesLeft, int fd)
{

    char buff[MAX_BUFFER_SIZE];
    int len = 0;

    if (*filesRead < filesLeft)
    {
        len = snprintf(buff, sizeof(buff),"%s", tasksRemaining[(*filesRead)++]);
        ERROR_CHECK(write(fd, buff, len+1), "Master - assignTask Write");
        return 1;
    }

    return 0;
}

int worksProcessed(char *buffer)
{
    return 0;
}

void replaceChar(char * buffer, char oldChar, char newChar){
    char * aux;
    while ((aux = strchr(buffer, oldChar)) != NULL)
        *aux = newChar;
}