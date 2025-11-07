

#include "shell.h"

/* Reads one line of input */
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

/* Tokenize a command string into arglist[] */
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
        while (*cp == ' ' || *cp == '\t') cp++;  // skip spaces
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

/* ================= Feature 2: Built-in Commands ================= */

int handle_builtin(char** arglist) {
    if (!arglist || !arglist[0]) return 0;

    // -------- EXIT --------
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // -------- CD --------
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

    // -------- HELP --------
    else if (strcmp(arglist[0], "help") == 0) {
        printf("\n--- myshell Help ---\n");
        printf("Built-in commands:\n");
        printf("  cd <dir>   - change current directory\n");
        printf("  exit       - exit the shell\n");
        printf("  help       - display this message\n");
        printf("  jobs       - list background jobs (Feature 6)\n");
        printf("---------------------\n\n");
        return 1;
    }

    // -------- JOBS (Placeholder) --------
    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not implemented yet (Feature 6).\n");
        return 1;
    }

    // -------- Not a Builtin --------
    return 0;
}
