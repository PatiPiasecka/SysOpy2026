/* - to usunac bo plik naglowkowy za to odpowiada
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
*/

#include "definitions.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {return 1;}
    int M = atoi(argv[1]);

    FILE *f = fopen(OUTPUT_FILE, "a");

    flock(fileno(f), LOCK_EX);

    for (int i = 0; i < M; i++) {
        fprintf(f, "Potomek (PID: %d)\n", getpid());
        fflush(f);
        usleep(250000);
    }

    flock(fileno(f), LOCK_UN);
    fclose(f);
    return 0;
}