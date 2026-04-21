#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <signal.h>

void sig_mask(){
    printf("Wywołano funkcję 'sig_mask()'\n");
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, NULL);

}