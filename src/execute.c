
#include "shell.h"

int execute(char* arglist[]) {
    int status;
    pid_t cpid = fork();

    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);

        case 0: // --- Child Process ---
            signal(SIGINT, SIG_DFL);  // restore Ctrl+C default
            execvp(arglist[0], arglist);
            perror("Command not found");
            exit(1);

        default: // --- Parent Process ---
            waitpid(cpid, &status, 0);
            return 0;
    }
}
