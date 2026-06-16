#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define BUF_SIZE 10
#define SYNC_TOLERANCE_MS 20

// klatka obrazu
typedef struct {
    int id;
    struct timespec timestamp;
} Frame;

// stan robota w przestrzeni
typedef struct {
    double x, y, theta; // x, y, kąt obrotu
    struct timespec timestamp; //czas odczytania pozycji
} RobotState;

// Bufor na zdjecia
typedef struct {
    Frame data[BUF_SIZE];
    int in, out;
    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
} FrameBuffer;

// Bufor na pozycje
typedef struct {
    RobotState data[BUF_SIZE];
    int in, out;
    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
} StateBuffer;

// extern - zmienne zewnętrzne 
extern FrameBuffer left_cam_buf, right_cam_buf, sync_buf;
extern StateBuffer robot_state_buf;

long time_diff_ms(struct timespec t1, struct timespec t2);
void sleep_until_next_period(struct timespec *next_activation, long period_ms);
void init_frame_buffer(FrameBuffer *buf);
void init_state_buffer(StateBuffer *buf);

#endif 