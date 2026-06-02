#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "chat.h"

// Statyczna tablica przechowująca otwarte kolejki zarejestrowanych klientów
mqd_t client_queues[MAX_CLIENTS];
int client_count = 0;

void handle_init(Message *msg) {
    if (client_count >= MAX_CLIENTS) {
        printf("Serwer: Osiągnięto limit klientów!\n");
        return;
    }

    // Otwieramy prywatną kolejkę nowo zgłoszonego klienta (tylko do zapisu)
    mqd_t cl_mq = mq_open(msg->client_queue_name, O_WRONLY);
    if (cl_mq == (mqd_t)-1) {
        perror("Serwer: Błąd otwierania kolejki klienta");
        return;
    }

    int new_id = client_count;
    client_queues[new_id] = cl_mq;
    client_count++;

    Message response;
    response.type = INIT;
    response.client_id = new_id;

    // Wysyłamy ID do klienta przez jego prywatną kolejkę
    mq_send(cl_mq, (const char*)&response, sizeof(Message), 0);
    printf("Serwer: Zarejestrowano klienta %d (Kolejka: %s)\n", new_id, msg->client_queue_name);
}

void handle_text(Message *msg) {
    printf("Serwer: Wiadomość od klienta %d: %s\n", msg->client_id, msg->text);

    // Przygotowanie paczki dla pozostałych uczestników chatu
    Message broadcast;
    broadcast.type = TEXT;
    broadcast.client_id = msg->client_id;
    snprintf(broadcast.text, sizeof(broadcast.text), "Klient %d: %.480s", msg->client_id, msg->text);

    // Roześlij do wszystkich zalogowanych klientów oprócz nadawcy
    for (int i = 0; i < client_count; i++) {
        if (i != msg->client_id) {
            mq_send(client_queues[i], (const char*)&broadcast, sizeof(Message), 0);
        }
    }
}

int main() {
    printf("--- SERWER CHATU ---\n");

    // Definiowanie nowej kolejki
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;           // Maksymalna liczba wiadomości w kolejce na raz
    attr.mq_msgsize = sizeof(Message); // Dokładny rozmiar jednej paczki danych
    attr.mq_curmsgs = 0;

    // usuwamy stary plik kolejki
    mq_unlink(SERVER_QUEUE);

    // Tworzenie głównej kolejki serwera (O_RDONLY - serwer tylko z niej czyta)
    mqd_t server_mq = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (server_mq == (mqd_t)-1) {
        perror("mq_open server");
        return 1;
    }

    printf("Uruchomiono. Czekam na komunikaty...\n");

    Message incoming_msg;
    
    while (1) {
        // mq_receive usypia serwer do momentu, aż pojawi się wiadomość
        if (mq_receive(server_mq, (char*)&incoming_msg, sizeof(Message), NULL) == -1) {
            perror("mq_receive server");
            break;
        }

        switch (incoming_msg.type) {
            case INIT:
                handle_init(&incoming_msg);
                break;
            case TEXT:
                handle_text(&incoming_msg);
                break;
            default:
                break;
        }
    }

    // Zamknięcie i usunięcie kolejki przy wyjściu
    for (int i = 0; i < client_count; i++) mq_close(client_queues[i]);
    mq_close(server_mq);
    mq_unlink(SERVER_QUEUE);
    return 0;
}