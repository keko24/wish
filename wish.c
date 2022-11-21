#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_LEN 80
#define MAX_BUFF_SIZE 1024
#define MAX_WORDS 100
#define MAX_PATHS 20 

int main(int argc, char *argv[]) {
    char *buffer, *token, *command[MAX_LEN], *searchPath[MAX_LEN];
    searchPath[0] = "/bin/";
    size_t bufsize = MAX_BUFF_SIZE;
    int wordsInCommand, searchPathLen = 1;
    pid_t child;
    buffer = (char *) malloc(bufsize);
    printf("wish> ");
    while (getline(&buffer, &bufsize, stdin) != -1) {
        wordsInCommand = 0;
        buffer[strlen(buffer) - 1] = '\0';
        while ((token = strsep(&buffer, " "))) {
            command[wordsInCommand++] = token; 
        }
        command[wordsInCommand] = NULL;
        if (strcmp(command[0], "exit") == 0) {
            exit(0);
        }
        else if (strcmp(command[0], "cd") == 0) {
            if (wordsInCommand < 2) {
                fprintf(stderr, "usage: cd <path-to_dir>\n");
            }
            chdir(command[1]);
        }
        else if (strcmp(command[0], "path") == 0) {
            searchPathLen = wordsInCommand - 1;
            for (int i = 0; i < searchPathLen; i++) {
                searchPath[i] = command[i + 1];
            }
        }
        else {
            child = fork();
            if (child == 0) {
                for (int i = 0; i < searchPathLen; i++) {
                    char path[MAX_LEN];
                    strcpy(path, searchPath[i]);
                    strcat(path, command[0]);
                    command[0] = path;
                    if (access(command[0], X_OK) == 0) {
                        break;
                    }
                }
                if (wordsInCommand > 2 && strcmp(command[wordsInCommand - 2], ">") == 0) {
                    freopen(command[wordsInCommand - 1], "w", stdout);
                    freopen(command[wordsInCommand - 1], "w", stderr);
                    command[wordsInCommand - 2] = NULL;
                }
                execv(command[0], command);
                fprintf(stderr, "The command %s failed to exexcute.\n", command[0]);
            }
            else if (child > 0) {
                waitpid(child, NULL, 0);
            }
            freopen("/dev/tty", "w", stdout);
            freopen("/dev/tty", "w", stderr);
        }
        printf("wish> ");
    }
    free(buffer);
    exit(0);
}
