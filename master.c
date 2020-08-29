#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define SLAVE "./slave"
#define MAX_SLAVES 5
#define PIPE_READ 0
#define PIPE_WRITE 1
#define ERROR_CHECK(x) do { \
  int retval = (x); \
  if (retval != 0) { \
    fprintf(stderr, "Runtime error: %s returned %d at %s:%d", #x, retval, __FILE__, __LINE__); \
    exit(-1); \
  } \
} while (0)

int createChildren();

typedef struct slave_t
{
    pid_t pid;
    int fdIn;
    int fdOut;
} slave_t;

slave_t children[MAX_SLAVES];

int main(int argc, char const *argv[])
{
    if( argc == 1){
        return 0;
    }

    createChildren();
    
    
    return 0;
}

int createChildren(){
    
    int id;
    char *arg[2]={SLAVE,(char*)NULL};


    for(int i = 0 ; i < MAX_SLAVES; i++){
        
        int pipeMtoS[2];
        int pipeStoM[2];

        
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

            execv(SLAVE,arg);
            
        }else{
            
            children[i].pid=id;
            children[i].fdIn = pipeMtoS[PIPE_WRITE];
            children[i].fdOut = pipeStoM[PIPE_READ];
            
            ERROR_CHECK(close(pipeMtoS[PIPE_READ]));
        
            ERROR_CHECK(close(pipeStoM[PIPE_WRITE]));
                
            
        }
        
    }
    
    return 0;
}
