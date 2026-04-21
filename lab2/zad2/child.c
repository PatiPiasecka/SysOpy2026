#define _POSIX_C_SOURCE 200809L //POSIX - standard C systemowy
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void handler(int signal_number){
    printf("Wywołano handler dla sygnału %d\n", signal_number);
}

void sig_default() {
    printf("Wywołano funkcję 'sig_default()'\n");
    signal(SIGUSR1, SIG_DFL);
}

void sig_mask(){
    printf("Wywołano funkcję 'sig_mask()'\n");
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, NULL);

}

void sig_unblock(){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void sig_ignore(){
    printf("Wywołano funkcję 'sig_ignore()'\n");
    signal(SIGUSR1, SIG_IGN);
}

void sig_handle(){
    printf("Wywołano funkcję 'sig_handle()'\n");
    signal(SIGUSR1, handler);
}

void handler_usr2(int sig, siginfo_t *info, void *context){
    int choice = info -> si_value.sival_int;
    if (choice == 1) sig_default();
    else if (choice == 2) sig_mask();
    else if (choice == 3) sig_ignore();
    else if (choice == 4) sig_handle();
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = handler_usr2;
    //blokujemy inne sygnaly
    sigemptyset(&sa.sa_mask);

    //otrzymywanie tego co wyslal rodzic przez sigqueue
    sa.sa_flags = SA_SIGINFO; 
    
    //pomaga przy handlerze do SIGUSR2
    sigaction(SIGUSR2, &sa, NULL);

    for (int i = 1; i <= 20; i++) {
        printf("%d\n", i);

        if(i==5 || i==15){
            printf("Wysyłam sygnał USR1\n");
            raise(SIGUSR1);
        }

        if (i==10){
            sigset_t pending;
            sigpending(&pending);
            if (sigismember(&pending, SIGUSR1)){
                printf("Odblokowuje USR1\n");
                sig_unblock();
            }
        }

        sleep(1);
    }

    printf("Petla zostala wykonana w calosci\n");
}