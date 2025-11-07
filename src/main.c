

#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    // Ignore Ctrl+C in parent shell
    signal(SIGINT, SIG_IGN);

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {

            // === Feature 2: Handle Built-ins ===
            if (!handle_builtin(arglist)) {
                // If not builtin, execute external command
                execute(arglist);
            }

            // === Free memory ===
            for (int i = 0; arglist[i] != NULL; i++) free(arglist[i]);
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
