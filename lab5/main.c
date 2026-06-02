#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define N 2 // Liczba klientow
#define M 2 // Liczba pracownikow
#define K 5 // Rozmiar bufora

typedef struct {
    char data[11];
} Task;

// Pamiec wspoldzielona
typedef struct {
    // ZAD2: Dwie oddzielne kolejki -> Normal i Priority
    Task normal_queue[K];
    int n_head, n_tail, n_count;

    Task prio_queue[K];
    int p_head, p_tail, p_count;

    // ZAD1: 3 semafory do synchronizacji
    sem_t empty; 
    sem_t full;
    sem_t mutex;
} SharedData;

// Funkcja do generowania losowego stringa
void generate_random_string(char *str, int length) {
    for (int i = 0; i < length; i++) {
        str[i] = 'A' + (rand() % 26);
    }
    str[length] = '\0';
}

void producer(SharedData *shared, int id) {
    srand(time(NULL) ^ (getpid() << 16)); 

    while (1) {
        char str[11];
        generate_random_string(str, 10);
        
        // ZAD2: 30% szans na trafienie do kolejki PRIORITY
        int is_priority = (rand() % 100) < 30; 

        // Sekwencja synchronizacji: czekaj na wolne miejsce, zablokuj bufor
        sem_wait(&shared->empty);
        sem_wait(&shared->mutex);

        // Zapis do odpowiedniej kolejki
        if (is_priority) {
            strcpy(shared->prio_queue[shared->p_tail].data, str);
            shared->p_tail = (shared->p_tail + 1) % K;
            shared->p_count++;
        } else {
            strcpy(shared->normal_queue[shared->n_tail].data, str);
            shared->n_tail = (shared->n_tail + 1) % K;
            shared->n_count++;
        }
        
        printf("[Producent %d] zadanie: %s -> %s\n", 
               id, str, is_priority ? "PRIORITY" : "NORMAL");

        // Zwolnij bufor, poinformuj o nowym elemencie
        sem_post(&shared->mutex);
        sem_post(&shared->full);

        sleep(1);
    }
}

void consumer(SharedData *shared, int id) {
    while (1) {
        sem_wait(&shared->full);
        sem_wait(&shared->mutex);

        Task task;
        int is_priority_task = 0;

        // ZAD2: Konsumenci zawsze najpierw sprawdzają kolejkę PRIORITY
        if (shared->p_count > 0) {
            task = shared->prio_queue[shared->p_head];
            shared->p_head = (shared->p_head + 1) % K;
            shared->p_count--;
            is_priority_task = 1;
        } else {
            // Jeśli PRIORITY pusta, bierz z NORMAL
            task = shared->normal_queue[shared->n_head];
            shared->n_head = (shared->n_head + 1) % K;
            shared->n_count--;
        }

        sem_post(&shared->mutex);
        sem_post(&shared->empty); 

        printf("[Konsument %d] pobral z %s. ", 
               id, is_priority_task ? "PRIORITY" : "NORMAL");
        
        // ZAD1: Wypisywanie znak po znaku z opóźnieniem
        for (int i = 0; i < 10; i++) {
            putchar(task.data[i]);
            fflush(stdout); 
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 300000000L; // 0.3s - 300 000 000 nanosekund
            nanosleep(&ts, NULL);
        }
        printf("\n");
    }
}

// MANAGER
void manager(SharedData *shared) {
    while (1) {
        sleep(5);

        sem_wait(&shared->mutex);
        
        printf("\n--- MANAGER ---\n");
        printf("Zadan w NORMAL: %d, Zadan w PRIORITY: %d\n", shared->n_count, shared->p_count);
        
        // Zapobieganie starvation - przeniesienie 1 zadania do PRIORITY
        if (shared->n_count > 0) {
            // Pobranie z NORMAL
            Task task = shared->normal_queue[shared->n_head];
            shared->n_head = (shared->n_head + 1) % K;
            shared->n_count--;
            
            // Umieszczenie w PRIORITY
            shared->prio_queue[shared->p_tail] = task;
            shared->p_tail = (shared->p_tail + 1) % K;
            shared->p_count++;
            
            printf("MANAGER: Podniesiono priorytet zadania %s (Normal -> Priority)\n", task.data);
        }
        
        sem_post(&shared->mutex);
    }
}


int main() {
    SharedData *shared = mmap(NULL, sizeof(SharedData), 
                              PROT_READ | PROT_WRITE, 
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }


    shared->n_head = shared->n_tail = shared->n_count = 0;
    shared->p_head = shared->p_tail = shared->p_count = 0;

    sem_init(&shared->empty, 1, K); // Pustych miejsc na start jest K
    sem_init(&shared->full, 1, 0);  // Zajętych miejsc jest 0
    sem_init(&shared->mutex, 1, 1); // Mutex pozwala 1 osobie wejść do strefy krytycznej

    // ZAD1: Tworzenie M konsumentów
    for (int i = 0; i < M; i++) {
        if (fork() == 0) {
            consumer(shared, i + 1);
            exit(0);
        }
    }

    // ZAD1: Tworzenie N producentów
    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            producer(shared, i + 1);
            exit(0);
        }
    }

    // ZAD3: Tworzenie 1 Managera
    if (fork() == 0) {
        manager(shared);
        exit(0);
    }

    for (int i = 0; i < N + M + 1; i++) {
        wait(NULL);
    }

    return 0;
}