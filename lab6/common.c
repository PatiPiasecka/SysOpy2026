#include "common.h"


FrameBuffer left_cam_buf, right_cam_buf, sync_buf;
StateBuffer robot_state_buf;

void init_frame_buffer(FrameBuffer *buf) {
    buf->in = 0; buf->out = 0;
    sem_init(&buf->empty, 0, BUF_SIZE);
    sem_init(&buf->full, 0, 0);
    pthread_mutex_init(&buf->mutex, NULL);
}

void init_state_buffer(StateBuffer *buf) {
    buf->in = 0; buf->out = 0;
    sem_init(&buf->empty, 0, BUF_SIZE);
    sem_init(&buf->full, 0, 0);
    pthread_mutex_init(&buf->mutex, NULL);
}

// Oblicza różnicę czasu w milisekundach
long time_diff_ms(struct timespec t1, struct timespec t2) {
    long ms1 = t1.tv_sec * 1000 + t1.tv_nsec / 1000000;
    long ms2 = t2.tv_sec * 1000 + t2.tv_nsec / 1000000;
    return labs(ms1 - ms2);
}

// sprawia aby kamera dzialala co 40ms
void sleep_until_next_period(struct timespec *next_activation, long period_ms) {
    next_activation->tv_nsec += period_ms * 1000000L;
    while (next_activation->tv_nsec >= 1000000000L) {
        next_activation->tv_nsec -= 1000000000L;
        next_activation->tv_sec++;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, next_activation, NULL);
}