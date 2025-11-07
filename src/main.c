

#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    signal(SIGINT, SIG_IGN);

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if (cmdline[0] == '\0') { free(cmdline); continue; }

        if (cmdline[0] == '!') {
            int n = atoi(cmdline + 1);
            char* prev = get_saved_command(n);
            if (!prev) {
                fprintf(stderr, "No such command: %d\n", n);
                free(cmdline);
                continue;
            }
            printf("Re-executing: %s\n", prev);
            free(cmdline);
            cmdline = strdup(prev);
        }

        save_command_history(cmdline);

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (!handle_builtin(arglist))
                execute(arglist);

            for (int i = 0; arglist[i] != NULL; i++)
                free(arglist[i]);
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
