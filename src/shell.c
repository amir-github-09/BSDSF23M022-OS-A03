

#include "shell.h"

/* -------------------- MANUAL HISTORY -------------------- */

static char* saved_history[HISTORY_SIZE];
static int history_count = 0;
static int history_next = 0;

void save_command_history(const char* cmd) {
    if (!cmd || cmd[0] == '\0') return;
    if (saved_history[history_next]) free(saved_history[history_next]);
    saved_history[history_next] = strdup(cmd);
    history_next = (history_next + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) history_count++;
}

void print_saved_history(void) {
    int start = (history_count < HISTORY_SIZE) ? 0 : history_next;
    int num = history_count;
    for (int i = 0; i < num; ++i) {
        int idx = (start + i) % HISTORY_SIZE;
        printf("%3d  %s\n", i + 1, saved_history[idx]);
    }
}

char* get_saved_command(int n) {
    if (n < 1 || n > history_count) return NULL;
    int start = (history_count < HISTORY_SIZE) ? 0 : history_next;
    int idx = (start + (n - 1)) % HISTORY_SIZE;
    return saved_history[idx];
}

/* -------------------- READLINE INPUT -------------------- */

char* read_cmd(char* prompt, FILE* fp) {
    (void)fp;
    char* line = readline(prompt);

    if (!line) return NULL;  // Ctrl-D

    // Add to GNU Readline internal history (for ↑/↓ arrows)
    if (strlen(line) > 0)
        add_history(line);  // <— This is from <readline/history.h>

    return line;
}

/* -------------------- TOKENIZE (unchanged) -------------------- */
char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0') return NULL;
    char** arglist = (char**) malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*) malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }
    char* cp = cmdline;
    int argnum = 0;
    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;
        char* start = cp;
        int len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) len++;
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }
    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }
    arglist[argnum] = NULL;
    return arglist;
}

/* -------------------- BUILTINS -------------------- */
int handle_builtin(char** arglist) {
    if (!arglist || !arglist[0]) return 0;

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }
    else if (strcmp(arglist[0], "cd") == 0) {
        if (!arglist[1]) {
            char* home = getenv("HOME");
            if (!home) fprintf(stderr, "cd: HOME not set\n");
            else if (chdir(home) != 0) perror("cd");
        } else {
            if (chdir(arglist[1]) != 0) perror("cd");
        }
        return 1;
    }
    else if (strcmp(arglist[0], "help") == 0) {
        printf("\nBuilt-ins: cd, exit, help, history, !n\n\n");
        return 1;
    }
    else if (strcmp(arglist[0], "history") == 0) {
        print_saved_history();
        return 1;
    }
    return 0;
}
