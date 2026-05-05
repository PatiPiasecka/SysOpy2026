#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "definitions.h"

double f(double x) {
    return 4.0 / (x * x + 1.0);
}

int main() {
    CalcRequest req;
    double result = 0.0;

    //otwieranie potoku do odczytu 
    int fd_req = open(FIFO_REQ, O_RDONLY);

    if (read(fd_req, &req, sizeof(CalcRequest)) > 0) {

        for (double x = req.a + req.dx / 2.0; x < req.b; x += req.dx) {
            result += f(x) * req.dx;
        }

        // Odsyłanie wyniku do main
        int fd_res = open(FIFO_RES, O_WRONLY);
        if (fd_res == -1) { perror("open FIFO_RES"); return 1; }
        write(fd_res, &result, sizeof(double));
        close(fd_res);
    }

    close(fd_req);
    return 0;
}