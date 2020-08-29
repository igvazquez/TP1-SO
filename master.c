#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define SLAVE "./slave"
#define MAX_SLAVES 5
#define PIPE_READ 0
#define PIPE_WRITE 1
#define PEND 5
#define MAX_BUFFER_SIZE 200
#define ERROR_CHECK(x) do { \
  int retval = (x); \
  if (retval != 0) { \
    fprintf(stderr, "Runtime error: %s returned %d at %s:%d", #x, retval, __FILE__, __LINE__); \
    exit(-1); \
  } \
} while (0)

int createChildren(slave_t children[],char *taskRemaining[],int * filesRead, int filesLeft);
int assingTask(char * taskRemaining[],int tasksToAssing,int * filesRead, int filesLeft, int fd);
int worksProcessed(char * buffer);

typedef struct slave_t
{
    pid_t pid;
    int fdIn;
    int fdOut;
    int pending;
} slave_t;


int main(int argc, char const *argv[])
{
    if( argc == 1){
        return 0;
    }

    int filesLeft = argc-1;
    int filesRead = 0, fdsAvailable = 0;
    int bytesRead=0;
    char * taskRemaining[] = argv+1; //salteo el primer argumento que es el ejecutable
    slave_t children[MAX_SLAVES];
    createChildren(children,taskRemaining,&filesRead,filesLeft);
    
    fd_set fdSet;
    char buffer[250];
    // Mientras queden archivos por leer
    while (filesRead < filesLeft){
        FD_ZERO(&fdSet);

        for (size_t i = 0; i < MAX_SLAVES; i++){
            FD_SET(children[i].fdOut , &fdSet);
        }

        ERROR_CHECK(fdsAvailable = select(children[MAX_SLAVES].fdOut + 1, &fdSet, NULL, NULL, NULL));

        for (size_t i = 0; i < MAX_SLAVES && fdsAvailable > 0; i++)
        {
            if (FD_ISSET(children[i].fdOut, &fdSet)){
               ERROR_CHECK(bytesRead+=read(children[i].fdOut,buffer,MAX_BUFFER_SIZE));// Leer los outputs del slave 
               worksProcessed(buffer);// Ver cuantas tareas hizo
                // Mandarle tantas tareas como las que ya realizo
                // Escribir lo que devolvio el hijo en el archivo de salida para view
                fdsAvailable--;
            }
        }
        
        
        
        

    }
    


    
    
    return 0;
}

int createChildren(slave_t children[],char *taskRemaining[],int * filesRead, int filesLeft){
    
    int id,aux;
    char *arg[2] = {SLAVE,(char*)NULL};
    
    for(int i = 0 ; i < MAX_SLAVES && *filesRead < filesLeft ; i++){
        
        int pipeMtoS[2];
        int pipeStoM[2];
        aux = 0;

        if(pipe(pipeMtoS) == -1)
            return 1; //TODO: error managing
        
        if(pipe(pipeStoM) == -1)
            return 1; //TODO: error managing
        
        if((id = fork()) == -1){
            return 2; //TODO: error managing
        }
        else if(id == 0){
            
            ERROR_CHECK(close(pipeMtoS[PIPE_WRITE]));
            ERROR_CHECK(close(pipeStoM[PIPE_READ]));
            ERROR_CHECK(dup2(pipeMtoS[PIPE_READ],STDIN_FILENO));
            ERROR_CHECK(dup2(pipeStoM[PIPE_WRITE],STDOUT_FILENO));
            ERROR_CHECK(close(pipeMtoS[PIPE_READ]));
            ERROR_CHECK(close(pipeStoM[PIPE_WRITE]));
            assignTask(taskRemaining,PEND,filesRead,filesLeft,children[i].fdIn);
            ERROR_CHECK(execv(SLAVE,arg)); // Solo se ejecuta error_check si execv falla
            
        }else{
            
            children[i].pid=id;
            children[i].fdIn  = pipeMtoS[PIPE_WRITE];
            children[i].fdOut = pipeStoM[PIPE_READ];
            children[i].pending = 1;
            
            ERROR_CHECK(close(pipeMtoS[PIPE_READ]));
            ERROR_CHECK(close(pipeStoM[PIPE_WRITE]));
                
            
        }
        
    }
    
    return 0;
}

int assingTask(char * taskRemaining[],int tasksToAssing,int * filesRead, int filesLeft, int fd){
    
    char buff[MAX_BUFFER_SIZE];

    for (size_t i = 0; i < tasksToAssing; i++){
        sprintf(buff, "%s\n", taskRemaining[*filesRead + i]);

    ERROR_CHECK(write(fd, buff, strlen(buff)));
        
    }
    
}

int worksProcessed(char * buffer){
    return 0;
}