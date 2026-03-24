#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int N = atoi(argv[1]);
    char *M_str = argv[2]; //child obsluzy dalej

    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            execlp("./child", "./child", M_str, NULL);
        }
    }

    while (wait(NULL) > 0);

    printf("Rodzic (PID: %d)\n", getpid());

    return 0;
}