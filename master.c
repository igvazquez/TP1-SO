#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define SLAVE "./slave"
#define MAX_SLAVES 5
#define PIPE_READ 0
#define PIPE_WRITE 1
#define PEND 5
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

typedef struct slave_t
{
    pid_t pid;
    int fdIn;
    int fdOut;
    int pending;
} slave_t;

int createChildren(slave_t children[], char *tasksRemaining[], int *filesRead, int filesLeft);
void assignTask(char *tasksRemaining[], int *filesRead, int filesLeft, int fd);
int worksProcessed(char *buffer);

int main(int argc, char const *argv[])
{
    if (argc == 1)
    {
        return 0;
    }

    int filesLeft = argc - 1;
    int filesRead = 0, fdsAvailable = 0, tasksDone = 0;
    int bytesRead = 0;
    char **tasksRemaining = argv + 1;
    slave_t children[MAX_SLAVES];
    int childrenCreated = createChildren(children, tasksRemaining, &filesRead, filesLeft);

    fd_set fdSet;
    char buffer[250];

    // bytesRead+=read(children[0].fdOut,buffer,MAX_BUFFER_SIZE);
    // buffer[bytesRead] = 0;
    // printf("%s\n", buffer);

    while (filesRead < filesLeft || tasksDone < filesLeft)
    {
        printf("llegue\n");
        FD_ZERO(&fdSet);

        for (size_t i = 0; i < childrenCreated; i++)
        {
            FD_SET(children[i].fdOut, &fdSet);
        }
        printf("hijes: %d\n", childrenCreated);
        printf("max fd: %d\n", children[childrenCreated - 1].fdOut);
        ERROR_CHECK(fdsAvailable = select(children[childrenCreated - 1].fdOut + 1, &fdSet, NULL, NULL, NULL), "Master - select");
        printf("sali del select\n");

        printf("%d\n", fdsAvailable);
        for (size_t i = 0; i < childrenCreated && fdsAvailable > 0; i++)
        {
            if (FD_ISSET(children[i].fdOut, &fdSet))
            {
                printf("llegue a hacer el read\n");
                ERROR_CHECK(bytesRead = read(children[i].fdOut, buffer, MAX_BUFFER_SIZE), "Master - read"); // Leer los outputs del slave
                printf("%s\n", buffer);
                tasksDone++;
                //worksProcessed(buffer);// Ver cuantas tareas hizo
                // Mandarle tantas tareas como las que ya realizo
                // Escribir lo que devolvio el hijo en el archivo de salida para view
                fdsAvailable--;
            }
            printf("sali de no hacer el read\n");
        }
    }

    return 0;
}

int createChildren(slave_t children[], char *tasksRemaining[], int *filesRead, int filesLeft)
{

    int i, id, pendings = 1;
    char *initArgsArray[INITIAL_TASKS + 2];

    if (filesLeft > MAX_SLAVES * 2)
    {
        pendings = 2;
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
        else{
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

void assignTask(char *tasksRemaining[], int *filesRead, int filesLeft, int fd)
{

    char buff[MAX_BUFFER_SIZE];
    int len = 0;

    if (*filesRead < filesLeft)
    {
        len = sprintf(buff, "%s\n", tasksRemaining[(*filesRead)++]);
        ERROR_CHECK(write(fd, buff, len), "Master - assignTask Write");
    }
}

int worksProcessed(char *buffer)
{
    return 0;
}