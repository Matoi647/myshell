#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX_CMD_LEN 512 // max length of a line of command
#define MAX_ARGC 20 // max number of arguments
#define MAX_ARG_LEN 50 // max length of an argument
#define MAX_CWD_LEN 512

char error_message[30] = "An error has ocurred\n";
char cwd[MAX_CWD_LEN];


// 在特殊字符左右添加空格
char* add_space(char *str, const char *delim) {
    int len = strlen(str);
    int delim_len = strlen(delim);
    char *result = (char*)malloc(2*len*sizeof(char));
    int i = 0;

    for (int j = 0; j < len; j++) {
        int is_delim = 0;
        for (int k = 0; k < delim_len; k++) {
            if (str[j] == delim[k]) {
                is_delim = 1;
                break;
            }
        }

        if (is_delim) {
            result[i++] = ' ';
            result[i++] = str[j];
            result[i++] = ' ';
        } else {
            result[i++] = str[j];
        }
    }

    result[i] = '\0';
    return result;
}

int parse_line(char* line, char** tokens)
{
    const char special_char[] = "<>&";
    char* cmd = add_space(line, special_char);
    const char delim[] = " ";
    int token_num = 0;

    char* token = strtok(cmd, delim);

    while (token != NULL && token_num < MAX_ARGC) {
        tokens[token_num++] = token;
        token = strtok(NULL, delim);
    }

    return token_num;
}

int redirection(char* argv[], char* inputFile, char* outputFile, int background)
{
    int rc = fork();
    if(rc < 0){
        perror("fork falied\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    } else if (rc == 0) {
        if (inputFile != NULL){
            int fd = open(inputFile, O_RDONLY, 0666);
            if (fd == -1){
                perror("open falied\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (outputFile != NULL){
            int fd = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
            if (fd == -1){
                perror("open falied\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(argv[0], argv);
    } else {
        if (background == 0){
            int rc_wait = wait(NULL);
        }
    }
    return 0;
}

int is_special_cmd(char* arg, char** special_cmd, int special_num){
    int res = 0;
    for(int i = 0;i < special_num; i++){
        if(strcmp(arg, special_cmd[i])==0){
            res = 1;
        }
    }
    return res;
}

int execute_command(char** argv, int background){
    int rc = fork();
    if (rc < 0){
        // fork falied
        perror("fork failed\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    } else if (rc == 0) {
        execvp(argv[0], argv);
        // if(execvp(argv[0], argv) == -1){
        //     // command not found
        //     write(STDERR_FILENO, error_message, strlen(error_message));
        // }
    } else {
        if (background == 0){
            int rc_wait = wait(NULL);
        }
    }
    return 0;
}

int handle_command(int argc, char** argv)
{
    if(argv[0] == NULL) {
        return 0;
    }
    char* atom_cmd[MAX_ARGC];   // command without special character
    memset(atom_cmd, 0, MAX_ARGC * sizeof(char*));
    int background = 0;

    char* special_cmd[3] = {"<", ">", "&"};
    int special_idx = -1;
    for(int i = 0; i < argc; i++){
        if(is_special_cmd(argv[i], special_cmd, 3)){
            special_idx = i;
            break;
        }
        atom_cmd[i] = argv[i];
    }
    // trailing ampersand
    if(strcmp(argv[argc-1], "&") == 0){
        background = 1;
    }
    // for(int i = 0; i < argc; i++){
    //     if(strcmp(argv[i], "&") == 0){
    //         background = 1;
    //     }
    // }

    if(strcmp(argv[0], "exit")==0){
        exit(0);
    } else if(strcmp(argv[0], "pwd")==0){
        // if redirection is used
        if(special_idx > 0){
            if(strcmp(argv[special_idx], ">") == 0 && argv[special_idx+1] != NULL) {
                int fd = open(argv[special_idx+1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                if (fd == -1){
                    perror("open falied\n");
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }
                int fd_stdout = dup(STDOUT_FILENO);
                // redirect to file
                dup2(fd, STDOUT_FILENO);
                close(fd);
                printf("%s\n", getcwd(cwd, MAX_CMD_LEN));
                // redirect back to stdout
                dup2(fd_stdout, STDOUT_FILENO);
            }
            //  else {
            //     // redirection command format error
            //     perror("redirection command format error\n");
            //     write(STDERR_FILENO, error_message, strlen(error_message));
            //     return 1;
            // }
        } else {
            printf("%s\n", getcwd(cwd, MAX_CMD_LEN));
        }
    } else if(strcmp(argv[0], "cd")==0){
        // if no argument for cd
        if (argv[1] == NULL){
            chdir(getenv("HOME"));
            // printf(getenv("HOME"));
        } else {
            if (chdir(argv[1]) == -1){
                // no such directory
                perror("no such directory\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                return -1;
            }
        }
    } else if(strcmp(argv[0], "wait")==0){
        
    } else {
        if(special_idx > 0){
            // redirection is used
            if(strcmp(argv[special_idx], "<") == 0){
                if(argv[special_idx + 1] != NULL
                    && argv[special_idx + 2] == NULL){
                    // input redirection
                    redirection(atom_cmd, argv[special_idx + 1], NULL, background);
                } else if (argv[special_idx + 1] != NULL 
                    && strcmp(argv[special_idx + 2], ">") == 0
                    && argv[special_idx + 3] != NULL) {
                    // input and out redirection
                    redirection(atom_cmd, argv[special_idx + 1], argv[special_idx + 3], background);
                }
            } else if (strcmp(argv[special_idx], ">") == 0) {
                // output redirection
                if (argv[special_idx + 1] != NULL){
                    redirection(atom_cmd, NULL, argv[special_idx + 1], background);
                }
            }
        } else {
            // redirection is not used
            execute_command(atom_cmd, background);
        }
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
        memset(line, 0, (MAX_CMD_LEN + 2) * sizeof(char));
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0'; // replace '\n' with '\0' (null)
        if (strlen(line) > MAX_CMD_LEN) {
            // too long command
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        // printf("len=%ld, %s\n", strlen(line), line);
        memset(tokens, 0, MAX_ARGC * sizeof(char*));
        int token_num = parse_line(line, tokens);
        // for (int i = 0; i < token_num; i++) {
        //     printf("Token %d: %s, len=%ld\n", i, tokens[i], strlen(tokens[i]));
        // }
        handle_command(token_num, tokens);
    }
    return 0;
}