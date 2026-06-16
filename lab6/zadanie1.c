#define _POSIX_C_SOURCE 200809L

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <math.h>
#include "common.h"

// WĄTKI KAMER
void* camera_thread(void* arg) {
    int is_left = *((int*)arg);
    FrameBuffer *buf = is_left ? &left_cam_buf : &right_cam_buf;
    int frame_id = 0;
    
    // sprawdzamy czas aby wiedziec od ktorego momentu odliczac
    struct timespec next_activation;
    clock_gettime(CLOCK_MONOTONIC, &next_activation);

    // nieskończona petla aby kamery caly czas dzialaly 
    while (1) {
        Frame f;
        f.id = ++frame_id;
        clock_gettime(CLOCK_MONOTONIC, &f.timestamp);

        // Zapis do bufora z ochroną
        sem_wait(&buf->empty);

        // gwarancja ze zaden inny watek nie zacznie czytac/modyfikowac 
        pthread_mutex_lock(&buf->mutex);
        
        buf->data[buf->in] = f;
        buf->in = (buf->in + 1) % BUF_SIZE;
        
        pthread_mutex_unlock(&buf->mutex);
        sem_post(&buf->full);

        sleep_until_next_period(&next_activation, 40);
    }
    return NULL;
}

// WĄTEK OBRAZÓW
void* sync_thread(void* arg) {
    (void)arg;
    while (1) {
        Frame left_f, right_f;

        // pobieranie zdjec z lewej kamery
        sem_wait(&left_cam_buf.full);
        pthread_mutex_lock(&left_cam_buf.mutex);
        left_f = left_cam_buf.data[left_cam_buf.out];
        left_cam_buf.out = (left_cam_buf.out + 1) % BUF_SIZE;
        pthread_mutex_unlock(&left_cam_buf.mutex);
        sem_post(&left_cam_buf.empty);

        // pobieranie zdjec z prawej kamery
        sem_wait(&right_cam_buf.full);
        pthread_mutex_lock(&right_cam_buf.mutex);
        right_f = right_cam_buf.data[right_cam_buf.out];
        right_cam_buf.out = (right_cam_buf.out + 1) % BUF_SIZE;
        pthread_mutex_unlock(&right_cam_buf.mutex);
        sem_post(&right_cam_buf.empty);

        // weryfikacja - sprawdza czy jest to para stereo
        if (time_diff_ms(left_f.timestamp, right_f.timestamp) < SYNC_TOLERANCE_MS) {
            sem_wait(&sync_buf.empty);
            pthread_mutex_lock(&sync_buf.mutex);
            sync_buf.data[sync_buf.in] = left_f; 
            sync_buf.in = (sync_buf.in + 1) % BUF_SIZE;
            pthread_mutex_unlock(&sync_buf.mutex);
            sem_post(&sync_buf.full);
        }
    }
    return NULL;
}


// WĄTEK ZAPISU OBRAZÓW 
void* writer_thread(void* arg) {
    (void)arg;
    struct timespec next_activation;
    clock_gettime(CLOCK_MONOTONIC, &next_activation);

    while (1) {
        Frame f;
        sem_wait(&sync_buf.full);
        pthread_mutex_lock(&sync_buf.mutex);
        f = sync_buf.data[sync_buf.out];
        sync_buf.out = (sync_buf.out + 1) % BUF_SIZE;
        pthread_mutex_unlock(&sync_buf.mutex);
        sem_post(&sync_buf.empty);

        // Tworzenie plików
        char filename_l[32], filename_r[32];
        snprintf(filename_l, sizeof(filename_l), "left_%04d.jpg", f.id);
        snprintf(filename_r, sizeof(filename_r), "right_%04d.jpg", f.id);

        FILE *fl = fopen(filename_l, "w"); if(fl) { fprintf(fl, "IMG\n"); fclose(fl); }
        FILE *fr = fopen(filename_r, "w"); if(fr) { fprintf(fr, "IMG\n"); fclose(fr); }
        
        printf("[ZADANIE 1] Zapisano parę nr: %d\n", f.id);

        sleep_until_next_period(&next_activation, 100);
    }
    return NULL;
}

// WĄTEK STANU ROBOTA 
void* robot_state_thread(void* arg) {
    (void)arg;
    struct timespec next_activation;
    clock_gettime(CLOCK_MONOTONIC, &next_activation);

    double current_x = 0.0;
    double current_y = 0.0;
    double theta = 0.0;

    double v = 0.5;
    double omega = 0.2;
    double dt = 0.01;

    while (1) {
        theta = theta + (omega*dt);

        if (theta > 2 * M_PI) {
            theta -= 2 * M_PI;
        }

        current_x = current_x + (v * cos(theta) * dt);
        current_y = current_y + (v * sin(theta) * dt);

        RobotState rs = {current_x, current_y, theta, next_activation};

        sem_wait(&robot_state_buf.empty);
        pthread_mutex_lock(&robot_state_buf.mutex);
        robot_state_buf.data[robot_state_buf.in] = rs;
        robot_state_buf.in = (robot_state_buf.in + 1) % BUF_SIZE;
        pthread_mutex_unlock(&robot_state_buf.mutex);
        sem_post(&robot_state_buf.full);

        sleep_until_next_period(&next_activation, 10);
    }
    return NULL;
}

// WĄTEK LOGGERA - sledzenie jak poruszal sie robot
void* logger_thread(void* arg) {
    (void)arg;
    struct timespec next_activation;
    clock_gettime(CLOCK_MONOTONIC, &next_activation);
    FILE *log_file = fopen("robot_state.log", "w");

    while (1) {
        RobotState rs;
        sem_wait(&robot_state_buf.full);
        pthread_mutex_lock(&robot_state_buf.mutex);
        rs = robot_state_buf.data[robot_state_buf.out];
        robot_state_buf.out = (robot_state_buf.out + 1) % BUF_SIZE;
        pthread_mutex_unlock(&robot_state_buf.mutex);
        sem_post(&robot_state_buf.empty);

        if (log_file) {
            fprintf(log_file, "T: %ld.%09ld | X:%.2f Y:%.2f\n", 
                    rs.timestamp.tv_sec, rs.timestamp.tv_nsec, rs.x, rs.y);
            fflush(log_file);
        }
        sleep_until_next_period(&next_activation, 100);
    }
    return NULL;
}

// MAIN
int main() {
    init_frame_buffer(&left_cam_buf);
    init_frame_buffer(&right_cam_buf);
    init_frame_buffer(&sync_buf);
    init_state_buffer(&robot_state_buf);

    pthread_t t_cam_l, t_cam_r, t_sync, t_writer, t_state, t_logger;
    int id_left = 1, id_right = 0;

    printf("URUCHOMIONO ZADANIE 1 \n");

    pthread_create(&t_cam_l, NULL, camera_thread, &id_left);
    pthread_create(&t_cam_r, NULL, camera_thread, &id_right);
    pthread_create(&t_sync, NULL, sync_thread, NULL);
    pthread_create(&t_writer, NULL, writer_thread, NULL);
    pthread_create(&t_state, NULL, robot_state_thread, NULL);
    pthread_create(&t_logger, NULL, logger_thread, NULL);

    pthread_exit(NULL);
}