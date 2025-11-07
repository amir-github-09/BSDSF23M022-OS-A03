

#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    signal(SIGINT, SIG_IGN);  // ignore Ctrl+C in shell

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if (cmdline[0] == '\0') { free(cmdline); continue; }

        /* ===== Feature 3: Handle !n re-execution ===== */
        if (cmdline[0] == '!') {
            int n = atoi(cmdline + 1);
            char* prev = get_history_command(n);
            if (prev == NULL) {
                fprintf(stderr, "No such command in history: %d\n", n);
                free(cmdline);
                continue;
            }
            printf("Re-executing: %s\n", prev);
            free(cmdline);
            cmdline = strdup(prev);  // use previous command line
        }

        /* Add command to history before tokenizing */
        add_history(cmdline);

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (!handle_builtin(arglist)) {
                execute(arglist);
            }

            for (int i = 0; arglist[i] != NULL; i++) free(arglist[i]);
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
