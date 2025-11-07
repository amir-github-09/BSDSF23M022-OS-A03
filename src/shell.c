

#include "shell.h"

/* -------------------- HISTORY STORAGE -------------------- */

static char* history[HISTORY_SIZE];
static int history_count = 0;    // total commands ever added
static int history_next = 0;     // next slot to write into (circular)

/* Add a command to history */
void add_history(const char* cmd) {
    if (cmd == NULL || cmd[0] == '\0') return;

    // Free old command in this slot
    if (history[history_next]) free(history[history_next]);
    history[history_next] = strdup(cmd);

    history_next = (history_next + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) history_count++;
}

/* Print all commands in history */
void print_history(void) {
    int start = (history_count < HISTORY_SIZE) ? 0 : history_next;
    int num = history_count;

    for (int i = 0; i < num; ++i) {
        int idx = (start + i) % HISTORY_SIZE;
        printf("%3d  %s\n", i + 1, history[idx]);
    }
}

/* Retrieve nth command (1-based) */
char* get_history_command(int n) {
    if (n < 1 || n > history_count) return NULL;

    int start = (history_count < HISTORY_SIZE) ? 0 : history_next;
    int idx = (start + (n - 1)) % HISTORY_SIZE;
    return history[idx];
}

/* -------------------- Input & Tokenization -------------------- */

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    fflush(stdout);

    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    if (!cmdline) {
        perror("malloc");
        return NULL;
    }

    int c, pos = 0;
    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        if (pos < MAX_LEN - 1) cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Ctrl+D
    }

    cmdline[pos] = '\0';
    return cmdline;
}

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

    // exit
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // cd
    else if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            char* home = getenv("HOME");
            if (home == NULL) fprintf(stderr, "cd: HOME not set\n");
            else if (chdir(home) != 0) perror("cd");
        } else {
            if (chdir(arglist[1]) != 0) perror("cd");
        }
        return 1;
    }

    // help
    else if (strcmp(arglist[0], "help") == 0) {
        printf("\n--- myshell Help ---\n");
        printf("Built-in commands:\n");
        printf("  cd <dir>    - change directory\n");
        printf("  exit        - exit shell\n");
        printf("  help        - show this help\n");
        printf("  jobs        - (Feature 6)\n");
        printf("  history     - show command history\n");
        printf("  !n          - execute nth command from history\n");
        printf("---------------------\n\n");
        return 1;
    }

    // jobs placeholder
    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not implemented yet (Feature 6).\n");
        return 1;
    }

    // Feature 3: history builtin
    else if (strcmp(arglist[0], "history") == 0) {
        print_history();
        return 1;
    }

    return 0;
}
