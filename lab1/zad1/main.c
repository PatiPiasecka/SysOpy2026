#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define M 5

int zmiennaGlobalna = 10;

int main(int argc, char *argv[]) {
    int N = atoi(argv[1]); // atoi - Ascii To Integer

    for (int i = 0; i < N; i++) {
        if (fork() == 0) { // vfork()
            zmiennaGlobalna++;
            
            for (int j = 0; j < M; j++) {
                printf("Potomek (%d)\n", getpid());
                usleep(250000); // sleep(0.25) - nie dziala poprawnie
            }
            exit(0);
        }
    }

    //czekaj na wszystkich potomkow
    while (wait(NULL) > 0);
    printf("Rodzic (%d) zmiennaGlobalna=%d\n", getpid(), zmiennaGlobalna);

    return 0;
}