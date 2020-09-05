// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define SLAVE "./slave.out"
#define _SVID_SOURCE 1
#define _POSIX_C_SOURCE 200112L
#define SEM_NAME "/IPC_Semaphore"
#define NAME "/IPC_Information"
#define MAX_SLAVES 5
#define PIPE_READ 0
#define PIPE_WRITE 1
#define MAX_BUFFER_SIZE 4096
#define INITIAL_TASKS 8
#define ERROR_CHECK(x, msg)                                                                              \
    do {                                                                                                 \
        int retval = (x);                                                                                \
        if (retval == -1) {                                                                              \
            fprintf(stderr, "Runtime error: %s returned %d at %s:%d\n", #x, retval, __FILE__, __LINE__); \
            perror(msg);                                                                                 \
            exit(-1);                                                                                    \
        }                                                                                                \
    } while (0)

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct slave_t {
    pid_t pid;
    int fdIn;
    int fdOut;
    int pending;
} slave_t;

int createChildren(slave_t children[], char *tasksRemaining[], int *filesRead, int filesLeft);
int assignTask(char *tasksRemaining[], int *filesRead, int filesLeft, int fd);
int worksProcessed(char *buffer);
void replaceChar(char *buffer, char oldChar, char newChar);
int createSharedMemory(int size, char **shm_base);
void closeSharedMemory(char *shm_base, int size, int shm_fd);
void closeFD(slave_t children[], int childrensCreated);
int writeOutput(char *output, FILE *outputFile, char *shm_ptr);

int main(int argc, char const *argv[]) {
    if (argc == 1) {
        return 0;
    }

    int filesLeft = argc - 1;
    int filesRead = 0, fdsAvailable = 0, tasksDone = 0, charsRead = 0;
    char **tasksRemaining = (char **)argv + 1;
    char *shm_base;
    char *ptr;
    slave_t children[MAX_SLAVES];

    sem_t *sem_id = sem_open(SEM_NAME, O_CREAT, 0666, 0);

    if (sem_id == SEM_FAILED) {
        ERROR_CHECK(-1, "Semaphore creation failed");
    }

    FILE *outputFile = fopen("output.txt", "w");

    int shm_size = filesLeft * MAX_BUFFER_SIZE;
    int shm_fd = createSharedMemory(shm_size, &shm_base);
    ptr = shm_base;

    if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
        ERROR_CHECK(-1, "Master - Setvbuf");

    int childrenCreated = createChildren(children, tasksRemaining, &filesRead, filesLeft);

    fd_set fdSet;
    char buffer[MAX_BUFFER_SIZE];
    char *token;
    char *delim = "\n";

    printf("%d", filesLeft);
    sleep(2);

    while (tasksDone < filesLeft) {
        FD_ZERO(&fdSet);

        for (size_t i = 0; i < childrenCreated; i++) {
            FD_SET(children[i].fdOut, &fdSet);
        }
        ERROR_CHECK(fdsAvailable = select(children[childrenCreated - 1].fdOut + 1, &fdSet, NULL, NULL, NULL), "Master - select");
        for (int i = 0; i < childrenCreated && fdsAvailable > 0; i++) {
            if (FD_ISSET(children[i].fdOut, &fdSet)) {
                ERROR_CHECK(charsRead = read(children[i].fdOut, buffer, MAX_BUFFER_SIZE - 1), "Master - read");  // Leer los outputs del slave
                buffer[charsRead] = 0;

                if (charsRead) {
                    token = strtok(buffer, delim);
                    while (token != NULL) {
                        replaceChar(token, '\t', '\n');
                        ptr += writeOutput(token, outputFile, ptr);
                        children[i].pending--;
                        tasksDone++;
                        ERROR_CHECK(sem_post(sem_id), "Master - sem post");
                        token = strtok(NULL, delim);
                    }

                    if (children[i].pending == 0) {
                        children[i].pending += assignTask(tasksRemaining, &filesRead, filesLeft, children[i].fdIn);
                    }
                }
                fdsAvailable--;
            }
        }
    }
    closeSharedMemory(shm_base, shm_size, shm_fd);
    closeFD(children, childrenCreated);
    fclose(outputFile);
    ERROR_CHECK(sem_close(sem_id), "Closing semaphore error");
    
    int state = sem_unlink(SEM_NAME);
    if(state != 0 && errno != ENOENT){
        ERROR_CHECK(-1,"Unlink semaphore error");
    }   
    
    return 0;
}

int writeOutput(char *output, FILE *outputFile, char *shm_ptr) {
    int len = strlen(output);

    if (fwrite(output, sizeof(char), len, outputFile) != len)
        ERROR_CHECK(-1, "Master - fwrite output file");

    memcpy((void *)shm_ptr, (void *)output, len + 1);
    return len + 1;
}

int createChildren(slave_t children[], char *tasksRemaining[], int *filesRead, int filesLeft) {
    int i, pendings = 1;
    char *initArgsArray[INITIAL_TASKS + 2];

    if (filesLeft > MAX_SLAVES * INITIAL_TASKS) {
        pendings = INITIAL_TASKS;
    }

    for (i = 0; i < MAX_SLAVES && *filesRead < filesLeft; i++) {
        int pipeMtoS[2];
        int pipeStoM[2];

        if (pipe(pipeMtoS) == -1)
            return 1;  //TODO: error managing

        if (pipe(pipeStoM) == -1)
            return 1;  //TODO: error managing

        int id;
        if ((id = fork()) == -1) {
            return 2;  //TODO: error managing
        } else if (id == 0) {
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
            ERROR_CHECK(execv(SLAVE, initArgsArray), "Master - execv");  // Solo se ejecuta error_check si execv falla
        } else {
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

int assignTask(char *tasksRemaining[], int *filesRead, int filesLeft, int fd) {

    if (*filesRead < filesLeft) {
        char buff[MAX_BUFFER_SIZE];
        int len = snprintf(buff, sizeof(buff), "%s", tasksRemaining[(*filesRead)++]);
        ERROR_CHECK(write(fd, buff, len + 1), "Master - assignTask Write");
        return 1;
    }

    return 0;
}

void replaceChar(char *buffer, char oldChar, char newChar) {
    char *aux;
    while ((aux = strchr(buffer, oldChar)) != NULL)
        *aux = newChar;
}

int createSharedMemory(int size, char **shm_base) {
    int shm_fd;
    ERROR_CHECK((shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666)), "Master - shm_open");

    ERROR_CHECK(ftruncate(shm_fd, size), "Master - ftruncate");

    *shm_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (*shm_base == MAP_FAILED)
        ERROR_CHECK(-1, "Master - mmap");

    return shm_fd;
}

void closeSharedMemory(char *shm_base, int size, int shm_fd) {
    ERROR_CHECK(munmap(shm_base, size), "Master - munmap");

    ERROR_CHECK(close(shm_fd), "Master - close shm");
}

void closeFD(slave_t children[], int childrensCreated) {
    for (int i = 0; i < childrensCreated; i++) {
        ERROR_CHECK(close(children[i].fdIn), "Master - close fdIn");
        ERROR_CHECK(close(children[i].fdOut), "Master - close fdOut");
    }
}