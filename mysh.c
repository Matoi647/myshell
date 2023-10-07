#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_CMD_LEN 512 // max length of a line of command
#define MAX_ARGC 20 // max number of arguments
#define MAX_ARG_LEN 30 // max length of an argument

char error_message[30] = "An error has ocurred\n";


// keep delim, remove white space
char* my_strtok(char* str, const char* delim) {
    static char* token;
    static char* nextToken;
    
    if (str != NULL) {
        token = str;
    } else {
        if (nextToken == NULL) {
            return NULL;
        }
        token = nextToken;
    }

    // Find the next occurrence of any delimiter character
    nextToken = token + strcspn(token, delim);

    if (*nextToken != '\0') {
        *nextToken = '\0';
        nextToken++;
    } else {
        nextToken = NULL;
    }

    return token;
}



int parse_command(char* cmd, char** tokens)
{
    const char delim[] = " <>&"; // TODO: distinguish < and <<
    int token_num = 0;
    char* token = my_strtok(cmd, delim);

    while (token != NULL && token_num < MAX_ARGC) {
        tokens[token_num++] = token;
        token = my_strtok(NULL, delim);
    }

    return token_num;
}

int do_command(int argc, char** argv)
{
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    } else if (rc == 0) {

        execvp(argv[0], argv);
    } else {
        int rc_wait = wait(NULL);
    }
    return 0;
}

int main(int argc, char** argv)
{
    char line[MAX_CMD_LEN + 2];
    char* tokens[MAX_ARGC];
    while (1) {
        // write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
        printf("mysh> ");
        memset(line, 0, MAX_CMD_LEN + 2);
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0'; // replace '\n' with '\0' (null)
        if (strlen(line) > MAX_CMD_LEN) {
            write(STDERR_FILENO, "too long command, ", strlen("too long command, "));
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        printf("len=%ld, %s\n", strlen(line), line);

        int token_num = parse_command(line, tokens);
        for (int i = 0; i < token_num; i++) {
            printf("Token %d: %s, len=%ld\n", i, tokens[i], strlen(tokens[i]));
        }
        do_command(token_num, tokens);
    }
    return 0;
}