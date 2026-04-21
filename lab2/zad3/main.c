#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int choice = 0;
    if (strcmp(argv[1], "default") == 0)      choice = 1;
    else if (strcmp(argv[1], "mask") == 0)    choice = 2;
    else if (strcmp(argv[1], "ignore") == 0)  choice = 3;
    else if (strcmp(argv[1], "handle") == 0)  choice = 4;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execl("./child", "./child", NULL);
        perror("execl");
        exit(1);
    } else {
        sleep(1); //gdyby rodzic nadal sygnal przed uruchomieniem dziecka
        union sigval sv;
        sv.sival_int = choice;

        if (sigqueue(pid, SIGUSR2, sv) == -1) {
            perror("sigqueue");
            return 1;
        }

        wait(NULL);
    }

    return 0;
}