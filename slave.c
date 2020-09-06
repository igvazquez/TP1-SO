// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _POSIX_C_SOURCE 2

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 2048
#define ERROR_CHECK(x, msg)                                                                              \
    do {                                                                                                 \
        int retval = (x);                                                                                \
        if (retval == -1) {                                                                              \
            fprintf(stderr, "Runtime error: %s returned %d at %s:%d\n", #x, retval, __FILE__, __LINE__); \
            perror(msg);                                                                                 \
            exit(-1);                                                                                    \
        }                                                                                                \
    } while (0)

void processTask(char *file);
void replaceChar(char *buffer, char oldChar, char newChar);

int main(int argc, char const *argv[]) {
    char buffer[BUFFER_SIZE];
    int len;

    if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
        ERROR_CHECK(-1, "Slave - Setvbuf");

    for (size_t i = 1; i < argc; i++)
        processTask((char *)argv[i]);

    while ((len = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1))) {
        if (len == -1)
            ERROR_CHECK(len, "Slave - read");
        buffer[len] = 0;
        processTask(buffer);
    }

    return 0;
}

void processTask(char *file) {
    char cmd[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    if (snprintf(cmd, sizeof(cmd), "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", file) < 0)
        ERROR_CHECK(-1, "Slave - snprintf");

    replaceChar(file, '\n', '\t');

    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
        ERROR_CHECK(-1, "Slave - popen");

    int len = fread(buffer, sizeof(char), sizeof(buffer) - 1, fp);
    buffer[len] = 0;

    replaceChar(buffer, '\n', '\t');

    if (ferror(fp))
        ERROR_CHECK(-1, "Slave - fread");

    printf("Nombre del archivo: %s\t%sID del esclavo: %d\t\n", file, buffer, getpid());

    ERROR_CHECK(pclose(fp), "Slave - pclose");
}

void replaceChar(char *buffer, char oldChar, char newChar) {
    char *aux;
    while ((aux = strchr(buffer, oldChar)) != NULL)
        *aux = newChar;
}