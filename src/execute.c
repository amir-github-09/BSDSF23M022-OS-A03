


#include "shell.h"

Job jobs[MAX_JOBS];
int job_count = 0;

/* -------------------- JOBS MANAGEMENT -------------------- */
void add_job(pid_t pid, const char* cmdline) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].cmdline, cmdline, MAX_LEN);
        jobs[job_count].active = 1;
        job_count++;
    }
}

void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid && jobs[i].active) {
            jobs[i].active = 0;
            printf("[+] Background job finished: PID=%d, CMD=%s\n", pid, jobs[i].cmdline);
        }
    }
}

void print_jobs(void) {
    printf("\nActive Background Jobs:\n");
    int any = 0;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active) {
            printf("[%d] PID=%d  CMD=%s\n", i + 1, jobs[i].pid, jobs[i].cmdline);
            any = 1;
        }
    }
    if (!any) printf("No background jobs.\n");
}

/* Reap terminated background processes */
void reap_terminated_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        remove_job(pid);
}

/* -------------------- EXECUTION (Handles Background) -------------------- */
int execute(char* arglist[], int background, const char* cmdline) {
    int status;
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    } 
    else if (pid == 0) {
        // Child process
        signal(SIGINT, SIG_DFL);
        execvp(arglist[0], arglist);
        perror("command failed");
        exit(1);
    } 
    else {
        if (background) {
            printf("[+] Background process started: PID=%d\n", pid);
            add_job(pid, cmdline);
            // Do not wait for it
            return 0;
        } else {
            waitpid(pid, &status, 0);
            return 0;
        }
    }
}
