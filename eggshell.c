#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

static char input_line[MAX_LINE];

void type_prompt(void);
void read_command(char **parameters);

int main(void) {
    char *parameters[MAX_ARGS];
    int status;

    while (1) {
        type_prompt();
        read_command(parameters);

        if (parameters[0] == NULL) {
            continue;
        }

        if (strcmp(parameters[0], "exit") == 0) {
            break;
        }

        if (strcmp(parameters[0], "cd") == 0) {
            char *target = parameters[1];

            if (target == NULL) {
                target = getenv("HOME");

                if (target == NULL) {
                    fprintf(stderr, "cd: HOME not set\n");
                    continue;
                }
            }

            if (chdir(target) != 0) {
                perror("cd");
            }

            continue;
        }

        int pipe_index = -1;
        for (int i = 0; parameters[i] != NULL; i++) {
            if (strcmp(parameters[i], "|") == 0) {
                pipe_index = i;
                break;
            }
        }

        if (pipe_index != -1) {

            if (pipe_index == 0)
            {
                fprintf(stderr, "Error: nothing before '|'\n");
                continue;
            }

            char *left_argv[MAX_ARGS];
            char *right_argv[MAX_ARGS];
            int i, j;

            for (i = 0; i < pipe_index; i++) {
                left_argv[i] = parameters[i];
            }
            left_argv[i] = NULL;

            j = 0;
            i = pipe_index + 1;
            if (parameters[i] == NULL) {
                fprintf(stderr, "syntax error: nothing after '|'\n");
                continue;
            }
            while (parameters[i] != NULL && j < MAX_ARGS - 1) {
                right_argv[j++] = parameters[i++];
            }
            right_argv[j] = NULL;

            int fd[2];
            if (pipe(fd) < 0) {
                perror("pipe");
                continue;
            }

            pid_t pid1 = fork();
            if (pid1 < 0) {
                perror("fork");
                close(fd[0]);
                close(fd[1]);
                continue;
            }

            if (pid1 == 0) {
                close(fd[0]);
                if (dup2(fd[1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    _exit(EXIT_FAILURE);
                }
                close(fd[1]);

                char fullpath[PATH_MAX];
                snprintf(fullpath, sizeof(fullpath), "/usr/bin/%s", left_argv[0]);
                execv(fullpath, left_argv);
                perror("execv");
                _exit(EXIT_FAILURE);
            }

            pid_t pid2 = fork();
            if (pid2 < 0) {
                perror("fork");
                close(fd[0]);
                close(fd[1]);
                waitpid(pid1, &status, 0);
                continue;
            }

            if (pid2 == 0) {
                close(fd[1]);
                if (dup2(fd[0], STDIN_FILENO) < 0) {
                    perror("dup2");
                    _exit(EXIT_FAILURE);
                }
                close(fd[0]);

                char fullpath[PATH_MAX];
                snprintf(fullpath, sizeof(fullpath), "/usr/bin/%s", right_argv[0]);
                execv(fullpath, right_argv);
                perror("execv");
                _exit(EXIT_FAILURE);
            }

            close(fd[0]);
            close(fd[1]);

            if (waitpid(pid1, &status, 0) < 0) {
                perror("waitpid");
            }
            if (waitpid(pid2, &status, 0) < 0) {
                perror("waitpid");
            }

            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }
        else if (pid == 0) {
            char fullpath[PATH_MAX];

            snprintf(fullpath, sizeof(fullpath), "/usr/bin/%s", parameters[0]);

            execv(fullpath, parameters);
            perror("execv");
            _exit(EXIT_FAILURE);
        }
        else {
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
            }
        }
    }

    return 0;
}

void type_prompt(void) {
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("eggshell:%s> ", cwd);
    } else {
        printf("eggshell> ");
    }

    fflush(stdout);
}

void read_command(char **parameters) {
    if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
        printf("\n");
        exit(EXIT_SUCCESS);
    }

    input_line[strcspn(input_line, "\n")] = '\0';

    int i = 0;
    char *token = strtok(input_line, " \t");

    while (token != NULL && i < MAX_ARGS - 1) {
        parameters[i++] = token;
        token = strtok(NULL, " \t");
    }

    parameters[i] = NULL;
}
