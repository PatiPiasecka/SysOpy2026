#include <stdio.h>
#include <signal.h>

void handler(int signal_number){
    printf("Wywołano handler dla sygnału %d\n", signal_number);
}

void sig_handle(){
    printf("Wywołano funkcję 'sig_handle()'\n");
    signal(SIGUSR1, handler);
}