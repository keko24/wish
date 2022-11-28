#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#define BUFF_SIZE 1000
#define MAX_LIST 100
#define MAX_COM 10

char error_message[30] = "An error has occurred\n";
char searchPath[MAX_COM][MAX_LIST] = {"/bin/"};
int searchPathLength = 1;

int take_input(char *string) {
    char *buffer;
    buffer = (char *) malloc(BUFF_SIZE);
    size_t buff_size = BUFF_SIZE;
    if (getline(&buffer, &buff_size, stdin) != -1) {
        buffer[strlen(buffer) - 1] = '\0';
        strcpy(string, buffer);
        free(buffer);
        return 0;
    }
    else {
        return 1;
    }
}

void redirect_to_file(char *file) {
    freopen(file, "w", stdout);
    freopen(file, "w", stderr);
}

void redirect_to_stdout() {
    freopen("/dev/tty", "w", stdout);
    freopen("/dev/tty", "w", stderr);
}

int built_in_functions(char **commands) {
    if (strcmp(commands[0], "exit") == 0) {                                                          
        exit(0);                                                                                        
    }                                                                                                   
    else if (strcmp(commands[0], "cd") == 0) {                                                       
        if (commands[1] == NULL) {                                                                       
            write(STDERR_FILENO, error_message, strlen(error_message));
        }                                                                                               
        chdir(commands[1]);
        return 1;
    }                                                                                                   
    else if (strcmp(commands[0], "path") == 0) {                                                     
        int i;
        for (i = 1; commands[i] != NULL; i++) {                                                       
            strcpy(searchPath[i - 1], commands[i]);
        }                                                                                               
        searchPathLength = i - 2;
        return 1;
    } 
    return 0;
}

void exec_command(char **command) {
    pid_t child;
    child = fork();
    if (child == 0) {
        execv(command[0], command);
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

void command_handler(char *parsedParallel[MAX_COM][MAX_LIST], int parallel) {
    for (int i = 0; i < parallel; i++) {
        if (!built_in_functions(parsedParallel[i])) {
            char path[MAX_LIST];
            int j;
            for (j = 0; searchPathLength; j++) {
                strcpy(path, searchPath[j]);
                strcat(path, parsedParallel[i][0]);
                if (!access(path, X_OK)) { 
                    parsedParallel[i][0] = path;
                    break;
                }
            }
            int check = -1, k;
            for (k = 0; parsedParallel[i][k] != NULL; k++) {
                if (strcmp(parsedParallel[i][k], ">") == 0) {
                    check = k;
                }
                else if (check != -1 && k > check + 1) {
                    check = -2;
                    break; 
                }
            }
            if (check == -2 || searchPathLength <= j) {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            else if (check > - 1) {
                parsedParallel[i][check] = NULL;
                redirect_to_file(parsedParallel[i][k - 1]);
                exec_command(parsedParallel[i]);
                redirect_to_stdout();
            }
            else {
                exec_command(parsedParallel[i]);
            }
        }
    } 
}

int parse_parallel(char *string, char **parallelCommands) {
    char *token;
    int i = 0;
    while ((token = strsep(&string, "&"))) {
        if (strlen(token) == 0) {
            continue;
        }
        parallelCommands[i++] = token;
    }
    return i;
}

void parse_spaces(char *command, char **parsedCommand) {
    char *token;
    int i = 0;
    while ((token = strsep(&command, " \t"))) {
        if (strlen(token) == 0)
            continue;
        parsedCommand[i++] = token;
    }
    parsedCommand[i] = NULL;
}


int process_string(char *string, char *parsedParallel[MAX_COM][MAX_LIST]) {
    char *stringParallel[MAX_LIST];
    int parallel = parse_parallel(string, stringParallel);
    for (int i = 0; i < parallel; i++) {
        parse_spaces(stringParallel[i], parsedParallel[i]);
    }
    return parallel;
}

int main(int argc, char *argv[]) {
    char inputString[BUFF_SIZE], *parsedParallel[MAX_COM][MAX_LIST];
    int parallel;
    printf("wish> ");
    while (1) {
        if (take_input(inputString)) {
            continue;
        }
        parallel = process_string(inputString, parsedParallel);
        command_handler(parsedParallel, parallel);
        while (wait(NULL) > 0);
        printf("wish> ");
    }
}
