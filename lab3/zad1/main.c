#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

double f(double x){
    return 4.0/(x*x + 1.0);
}

int main(int argc, char *argv[]) {
    double dx = atof(argv[1]);
    int n = atoi(argv[2]);
    long long total_steps = (long long)(1.0 / dx);


    for (int k=1; k<=n; k++){
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        int fd[k][2];
        //dla kazdego elementu kanał komunikacji
        for (int i = 0; i < k; i++) {
            if (pipe(fd[i]) == -1) {
                perror("pipe");
                exit(1);
            }
        }
        
        //tworzenie k procesów potomnych
        for (int i= 0; i<k; i++){
            pid_t pid = fork();

            if (pid<0){
                perror("fork");
                exit(1);
            }

            if (pid==0){
                //zamykanie koncow odczytu potokow
                for (int j=0; j<k; j++) close(fd[j][0]);
                //zamykanie koncow zapisu innych procesow
                for (int j=0; j<k; j++) if (j != i) close(fd[j][1]);

                double part_sum = 0.0;
                //kazdy proces liczy co k-ty prostokat
                for(long long j = i; j<total_steps; j+=k){
                    //srodkowy punkt prostokata
                    double x = (j+0.5)*dx;
                    part_sum += f(x)* dx;
                }

                write(fd[i][1], &part_sum, sizeof(double));
                close(fd[i][1]);
                exit(0);
            }
        }

        //macierzysty
        double total_result = 0.0;
        for (int i=0; i<k; i++){
            double child_result;
            close(fd[i][1]);
            read(fd[i][0], &child_result, sizeof(double));
            total_result += child_result;
            close(fd[i][0]);
        }

        //czekam na koniec wszystkich dzieci
        for (int i = 0; i < k; i++) wait(NULL);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time_spent = (end.tv_sec - start.tv_sec) + 
                           (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("k = %d | Wynik: %.10f | Czas: %.6f s\n", k, total_result, time_spent);
                        }
    return 0;
}