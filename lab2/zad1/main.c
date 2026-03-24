#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void sig_default() {
    signal(SIGUSR1, SIG_DFL);
}

void sig_mask(){
    return 0;
}


int main(int argc, char *argv[]) {
    for (int i = 1; i <= 20; i++) {
        printf("i = %d\n", i);
        sleep(1);
    }

}