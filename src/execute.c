



#include "shell.h"

// Helper: find the index of special symbols in arglist
int find_symbol(char** arglist, const char* symbol) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], symbol) == 0)
            return i;
    }
    return -1;
}

// Helper: split args around a symbol like '|'
void split_args(char** arglist, int index, char** left, char** right) {
    int i;
    for (i = 0; i < index; i++)
        left[i] = arglist[i];
    left[i] = NULL;

    int j = 0;
    for (i = index + 1; arglist[i] != NULL; i++)
        right[j++] = arglist[i];
    right[j] = NULL;
}

/* ========================= MAIN EXECUTE ========================= */
int execute(char* arglist[]) {
    int input_index = find_symbol(arglist, "<");
    int output_index = find_symbol(arglist, ">");
    int append_index = find_symbol(arglist, ">>");
    int pipe_index = find_symbol(arglist, "|");

    // ---------- Case 1: Handle PIPE (cmd1 | cmd2) ----------
    if (pipe_index != -1) {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe failed");
            return -1;
        }

        char* left[MAXARGS + 1];
        char* right[MAXARGS + 1];
        split_args(arglist, pipe_index, left, right);

        pid_t pid1 = fork();
        if (pid1 == 0) {
            // Child 1: send output to pipe
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            execvp(left[0], left);
            perror("pipe: command1 failed");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            // Child 2: receive input from pipe
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            execvp(right[0], right);
            perror("pipe: command2 failed");
            exit(1);
        }

        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return 0;
    }

    // ---------- Case 2: Handle Output Redirection ----------
    int saved_stdout = -1;
    if (output_index != -1 || append_index != -1) {
        int idx = (output_index != -1) ? output_index : append_index;
        char* filename = arglist[idx + 1];
        if (!filename) {
            fprintf(stderr, "syntax error: no file after > or >>\n");
            return -1;
        }

        int flags = (output_index != -1)
                    ? (O_WRONLY | O_CREAT | O_TRUNC)
                    : (O_WRONLY | O_CREAT | O_APPEND);

        int fd = open(filename, flags, 0644);
        if (fd == -1) {
            perror("open");
            return -1;
        }

        saved_stdout = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        arglist[idx] = NULL; // cut off redirection symbols
    }

    // ---------- Case 3: Handle Input Redirection ----------
    int saved_stdin = -1;
    if (input_index != -1) {
        char* filename = arglist[input_index + 1];
        if (!filename) {
            fprintf(stderr, "syntax error: no file after <\n");
            return -1;
        }

        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("open");
            return -1;
        }

        saved_stdin = dup(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
        close(fd);
        arglist[input_index] = NULL;
    }

    // ---------- Normal command execution ----------
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        execvp(arglist[0], arglist);
        perror("command not found");
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
    }

    // ---------- Restore file descriptors ----------
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }

    return 0;
}
