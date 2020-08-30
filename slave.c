#include <unistd.h>
#include <stdio.h>

#define BUFFER_SIZE 200
#define FILE_SIZE 50

int main(int argc, char const *argv[])
{
    char buffer[BUFFER_SIZE];
    char file[FILE_SIZE];
    int len;
    int fd;
    while( (len = read(STDIN_FILENO,buffer,BUFFER_SIZE) ) > 0){

        write(STDOUT_FILENO,buffer,BUFFER_SIZE);



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
        }

    return 0;
    }



