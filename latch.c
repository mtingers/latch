/*
 * latch.c
 * author: Matth Ingersoll <matth@mtingers.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>

unsigned int get_nprocs()
{
    /* scan /proc for pid dirs */
    DIR *dp;
    struct dirent *de;
    unsigned int nprocs = 0;

    dp = opendir ("/proc");
    if(!dp) {
        perror("opendir");
        return 0;
    }
    while((de = readdir(dp))) {
        if(isdigit(de->d_name[0]))
            nprocs++;
    }
    closedir(dp);
    return nprocs;
}
struct proc_info {
    FILE *f;
    pid_t pid;
};

struct proc_info *vmstat_init()
{
    struct proc_info *pf = malloc(sizeof(*pf));
    int pfd[2];

    if(!pf) {
        perror("malloc");
        return NULL;
    }
    if(pipe(pfd) == -1) {
        perror("pipe()");
        return NULL;
    }

    pf->pid = fork();
    if(pf->pid == 0) {
        // child
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        dup2(pfd[1], STDERR_FILENO);
        execl("/usr/bin/vmstat", "/usr/bin/vmstat", "-n", "1", (char *)NULL);
    }
    close(pfd[1]);
    pf->f = fdopen(pfd[0], "r");
    return pf;
}

struct proc_info *ifstat_init()
{
    struct proc_info *pf = malloc(sizeof(*pf));
    int pfd[2];

    if(!pf) {
        perror("malloc");
        return NULL;
    }
    if(pipe(pfd) == -1) {
        perror("pipe()");
        return NULL;
    }

    pf->pid = fork();
    if(pf->pid == 0) {
        // child
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        dup2(pfd[1], STDERR_FILENO);
        execl("/usr/bin/ifstat", "/usr/bin/ifstat", "-n", "-w", "-b", (char *)NULL);
    }
    close(pfd[1]);
    pf->f = fdopen(pfd[0], "r");
    return pf;
}

char *rtrim(const char *s)
{
    char *p = (char *)s;
    while(*p != '\0' && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')) {
        p++;
    }
    return p;
}

void print_vmstat_json(const char *s)
{
    //rhat:   r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
    //debian: r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa

    char *p = (char *)s;
    int buf_i = 0;
    int sts_pos = 0;
    char buf[64] = {0};
    char *sts[] = {
        "r", "b", "swpd", "free", "buff", "cache", "si", "so",
        "bi", "bo", "in", "cs", "us", "sy", "id", "wa", "st"
    };

    printf("\"vmstat\": {");
    while(*p != '\0' && *p!= '\n' && sts_pos < 18) {
        while(*p != '\0' && *p!= '\n' && *p == ' ')
            p++;
        buf_i = 0;
        while(*p != '\0' && *p!= '\n' && *p != ' ' && buf_i < 63) {
            buf[buf_i++] = *p;
            p++;
        }
        buf[buf_i] = '\0';
        printf("\"%s\":\"%s\", ", sts[sts_pos], buf);
        sts_pos++;
    }
    // no st in output
    if(sts_pos == 16)
        printf("\"st\":\"0\", ");
    printf("\"nop\":\"0\" }");
}

void print_ifstat_json(const char *s)
{
    char *p = (char *)s;
    int interface = 0;
    int buf_i = 0;
    int x = 0;
    char buf[64];

    printf("\"ifstat\": {");
    while(*p != '\0' && *p!= '\n') {
        while(*p != '\0' && *p!= '\n' && *p == ' ')
            p++;
        buf_i = 0;
        while(*p != '\0' && *p!= '\n' && *p != ' ' && buf_i < 63) {
            buf[buf_i++] = *p;
            p++;
        }
        buf[buf_i] = '\0';
        if(x == 0) {
            printf("\"interface%d_in\":\"%s\", ", interface, buf);
            x = 1;
        } else {
            printf("\"interface%d_out\":\"%s\", ", interface, buf);
            x = 0;
            interface++;
        }
    }
    printf("\"nop\":\"0\" }");
}

int main(int argc, char **argv)
{
    int status;
    char vmstat_line[256];
    char ifstat_line[512];
    char *vmstat_p = NULL, *ifstat_p = NULL;
    struct proc_info *vmstat_proc = vmstat_init();
    struct proc_info *ifstat_proc = NULL;

    if(!vmstat_proc) {
        printf("ERROR: Failed to create vmstat process!\n");
        return 1;
    }
    if(!(ifstat_proc = ifstat_init())) {
        printf("ERROR: Failed to create ifstat process!\n");
        return 1;
    }
    /* MAIN LOOP */
    while(1) {
        if(!fgets(vmstat_line, sizeof(vmstat_line), vmstat_proc->f))
            break;
        if(!fgets(ifstat_line, sizeof(ifstat_line), ifstat_proc->f))
            break;
        vmstat_p = rtrim(vmstat_line);
        ifstat_p = rtrim(ifstat_line);
        if(!isdigit(vmstat_p[0]) || !isdigit(ifstat_p[0]))
            continue;
        printf("{ \"timestamp\":\"%lld\", ", (long long)time(NULL));
        print_ifstat_json(ifstat_p);
        printf(", ");
        print_vmstat_json(vmstat_p);
        printf(" }\n");
        fflush(stdout);
    }

    printf("Wait for processes to close...\n");
    waitpid(vmstat_proc->pid, &status, 0);
    waitpid(ifstat_proc->pid, &status, 0);

    return 0;
}
