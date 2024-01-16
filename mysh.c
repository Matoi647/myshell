#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_CMD_LEN 512 // max length of a line of command
#define MAX_ARGC 20     // max number of arguments
#define MAX_ARG_LEN 50  // max length of an argument
#define MAX_CWD_LEN 512

char error_message[30] = "An error has occurred\n";
char cwd[MAX_CWD_LEN];

// add space near special character
char* add_space(char* str, const char* delim)
{
    int len = strlen(str);
    int delim_len = strlen(delim);
    char* result = (char*)malloc(2 * len * sizeof(char));
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
    const char delim[] = " \t\n\r";
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
    int pid = fork();
    if (pid < 0) {
        // perror("fork falied\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    } else if (pid == 0) {
        if (inputFile != NULL) {
            int fd = open(inputFile, O_RDONLY, 0666);
            if (fd == -1) {
                // perror("open falied\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (outputFile != NULL) {
            int fd = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
            if (fd == -1) {
                // perror("open falied\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (execvp(argv[0], argv) == -1) {
            // perror("subprocess command execution falied\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    } else {
        if (background == 0) {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

int is_special_cmd(char* arg, char** special_cmd, int special_num)
{
    int res = 0;
    for (int i = 0; i < special_num; i++) {
        if (strcmp(arg, special_cmd[i]) == 0) {
            res = 1;
        }
    }
    return res;
}

int execute_command(char** argv, int background)
{
    int pid = fork();
    if (pid < 0) {
        // perror("fork failed\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    } else if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            // perror("subprocess command execution falied\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    } else {
        if (background == 0) {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

int handle_command(int argc, char** argv)
{
    if (argv[0] == NULL) {
        return 0;
    }

    char* atom_cmd[MAX_ARGC]; // command without special character
    memset(atom_cmd, 0, MAX_ARGC * sizeof(char*));
    int background = 0;

    char* special_char[3] = { "<", ">", "&" };
    for (int i = 0; i < argc; i++) {
        if (is_special_cmd(argv[i], special_char, 3)) {
            break;
        }
        atom_cmd[i] = argv[i];
    }

    int redir_idx = -1;   // redirection character index
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">") == 0) {
            redir_idx = i;
            break;
        }
    }

    // trailing ampersand
    if (strcmp(argv[argc - 1], "&") == 0) {
        background = 1;
    }

    if (strcmp(argv[0], "exit") == 0) {
        if (argv[1] == NULL) {
            exit(0);
        } else {
            // perror("exit falied\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
        }

    } else if (strcmp(argv[0], "pwd") == 0) {
        if (argv[1] == NULL) {
            write(STDOUT_FILENO, getcwd(cwd, MAX_CMD_LEN), strlen(getcwd(cwd, MAX_CMD_LEN)));
            write(STDOUT_FILENO, "\n", strlen("\n"));
        } else {
            // perror("pwd command format error\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    } else if (strcmp(argv[0], "cd") == 0) {
        // if no argument for cd
        if (argv[1] == NULL) {
            chdir(getenv("HOME"));
            // printf(getenv("HOME"));
        } else if (argv[1] != NULL && argv[2] == NULL) {
            if (chdir(argv[1]) == -1) {
                // perror("no such directory\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        } else {
            // perror("cd command format error\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    } else if (strcmp(argv[0], "wait") == 0) {
        if (argv[1] == NULL) {
            int status;
            int wpid = wait(&status);
            while (wpid > 0) {
                wpid = wait(&status);
            }
        } else {
            // perror("wait command format error\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
        }

    } else {
        if (redir_idx > 0) {
            // redirection is used
            if (strcmp(argv[redir_idx], "<") == 0) {
                if (argv[redir_idx + 1] != NULL
                    && (argv[redir_idx + 2] == NULL
                        || strcmp(argv[redir_idx + 2], "&") == 0)) {
                    // input redirection
                    redirection(atom_cmd, argv[redir_idx + 1], NULL, background);
                } else if (argv[redir_idx + 1] != NULL
                    && strcmp(argv[redir_idx + 2], ">") == 0
                    && argv[redir_idx + 3] != NULL
                    && (argv[redir_idx + 4] == NULL
                        || strcmp(argv[redir_idx + 4], "&") == 0)) {
                    // input and out redirection
                    redirection(atom_cmd, argv[redir_idx + 1], argv[redir_idx + 3], background);
                } else {
                    // redirection format error
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            } else if (strcmp(argv[redir_idx], ">") == 0) {
                if (argv[redir_idx + 1] != NULL
                    && (argv[redir_idx + 2] == NULL
                        || strcmp(argv[redir_idx + 2], "&") == 0)) {
                    // output redirection
                    redirection(atom_cmd, NULL, argv[redir_idx + 1], background);
                } else {
                    // redirection format error
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
        } else {
            // redirection is not used
            execute_command(atom_cmd, background);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc > 1) {
        // batch mode
        char* filename = argv[1];
        if (filename != NULL) {
            // input redirection
            int fd = open(filename, O_RDONLY, 0666);
            if (fd == -1) {
                // perror("open falied\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);

            char line[MAX_CMD_LEN + 2];
            char* tokens[MAX_ARGC];
            memset(line, 0, (MAX_CMD_LEN + 2) * sizeof(char));
            memset(tokens, 0, MAX_ARGC * sizeof(char*));
            while (fgets(line, sizeof(line), stdin) != NULL) {
                // write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
                // printf("mysh> ");
                write(STDOUT_FILENO, line, strlen(line));

                line[strcspn(line, "\n")] = '\0'; // replace '\n' with '\0' (null)
                if (strlen(line) > MAX_CMD_LEN) {
                    // too long command
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }

                int token_num = parse_line(line, tokens);
                handle_command(token_num, tokens);
                memset(line, 0, (MAX_CMD_LEN + 2) * sizeof(char));
                memset(tokens, 0, MAX_ARGC * sizeof(char*));
            }
        }
    } else {
        // interactive mode
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

            memset(tokens, 0, MAX_ARGC * sizeof(char*));
            int token_num = parse_line(line, tokens);
            handle_command(token_num, tokens);
        }
    }

    return 0;
}