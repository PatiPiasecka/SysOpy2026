#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "definitions.h"

int main() {
    CalcRequest req;
    double result;

    printf("Podaj poczatek przedzialu: ");
    scanf("%lf", &req.a);
    printf("Podaj koniec przedzialu: ");
    scanf("%lf", &req.b);
    printf("Podaj dokladnosc: ");
    scanf("%lf", &req.dx);

    //Tworzenie potoków nazwanych
    //odczyt/zapis
    mkfifo(FIFO_REQ, 0666);
    mkfifo(FIFO_RES, 0666);

    // wysyłanie zapytania do Workera
    // open() zablokuje się dopoki Worker nie otworzy FIFO do odczytu
    int fd_req = open(FIFO_REQ, O_WRONLY);
    if (fd_req == -1) { perror("open FIFO_REQ"); return 1; }
    write(fd_req, &req, sizeof(CalcRequest));
    close(fd_req);

    //odbieranie wyniku
    int fd_res = open(FIFO_RES, O_RDONLY);
    if (fd_res == -1) { perror("open FIFO_RES"); return 1; }
    read(fd_res, &result, sizeof(double));
    close(fd_res);

    printf("\nWynik otrzymany z potoku: %.10f\n", result);

    //usuwanie potoków z systemu plików
    unlink(FIFO_REQ);
    unlink(FIFO_RES);

    return 0;
}