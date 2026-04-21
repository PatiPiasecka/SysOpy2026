#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#ifdef DYNAMIC_LOAD
    #include <dlfcn.h>
    void sig_unblock(); 
#else
    #include "signals.h" 
#endif

void handler_usr2(int sig, siginfo_t *info, void *context) {
    int choice = info->si_value.sival_int;

#ifdef DYNAMIC_LOAD
    char *symbol_name = NULL;
    if (choice == 1) symbol_name = "sig_default";
    else if (choice == 2) symbol_name = "sig_mask";
    else if (choice == 3) symbol_name = "sig_ignore";
    else if (choice == 4) symbol_name = "sig_handle";
    
    if (!symbol_name) return;

    //plik biblioteki
    void *handle = dlopen("./libsignals.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Błąd dlopen: %s\n", dlerror());
        return;
    }

    dlerror();
    void (*func)() = (void (*)())dlsym(handle, symbol_name);
    
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Błąd dlsym: %s\n", error);
    } else if (func) {
        func(); 
    }

    //zamykam biblioteke
    dlclose(handle);
#else
    if (choice == 1) sig_default();
    else if (choice == 2) sig_mask();
    else if (choice == 3) sig_ignore();
    else if (choice == 4) sig_handle();
#endif
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = handler_usr2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO; 
    
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