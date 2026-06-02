#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include "chat.h"

int main() {
    char my_queue_name[64];
    int my_id = -1;

    snprintf(my_queue_name, sizeof(my_queue_name), "/client_queue_%d", getpid());

    printf("--- KLIENT CHATU ---\n");

    // Otwarcie kolejki serwera (klient tylko do niej zapisuje - O_WRONLY)
    mqd_t server_mq = mq_open(SERVER_QUEUE, O_WRONLY);
    if (server_mq == (mqd_t)-1) {
        perror("Klient: Brak połączenia z serwerem");
        return 1;
    }

    // Prywatna kolejka do odbierania wiadomości od serwera
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Message);
    attr.mq_curmsgs = 0;

    mq_unlink(my_queue_name); // gdyby plik o tej nazwie istniał
    mqd_t my_mq = mq_open(my_queue_name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (my_mq == (mqd_t)-1) {
        perror("Klient: Błąd tworzenia prywatnej kolejki");
        mq_close(server_mq);
        return 1;
    }

    // zgłoszenie INIT do serwera
    Message init_msg;
    init_msg.type = INIT;
    strcpy(init_msg.client_queue_name, my_queue_name);
    mq_send(server_mq, (const char*)&init_msg, sizeof(Message), 0);

    // Oczekiwanie na odpowiedź serwera z przydzielonym ID
    Message init_response;
    mq_receive(my_mq, (char*)&init_response, sizeof(Message), NULL);
    my_id = init_response.client_id;
    
    printf("Połączono z chatem! Twoje ID to: %d\n", my_id);
    printf("Wpisz wiadomość i naciśnij Enter (Ctrl+D aby wyjść):\n-----------\n");

    //Rozdzielenie klienta na dwa procesy
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // PROCES POTOMNY (DZIECKO)
        // Odbieranie wiadomości z serwera i wypisywaniem ich
        Message incoming;
        while (1) {
            if (mq_receive(my_mq, (char*)&incoming, sizeof(Message), NULL) != -1) {
                if (incoming.type == TEXT) {
                    printf("\n%s\n", incoming.text);
                    printf("> ");
                    fflush(stdout);
                }
            }
        }
        exit(0);
    } 
    else {
        // PROCES MACIERZYSTY (RODZIC)
        // Odczyt ze standardowego wejścia i wysyłaniem do serwera
        char buffer[MAX_MSG_SIZE];
        Message text_msg;
        text_msg.type = TEXT;
        text_msg.client_id = my_id;

        printf("> ");
        fflush(stdout);

        while (fgets(buffer, MAX_MSG_SIZE, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;

            if (strlen(buffer) > 0) {
                strcpy(text_msg.text, buffer);
                mq_send(server_mq, (const char*)&text_msg, sizeof(Message), 0);
            }
            printf("> ");
            fflush(stdout);
        }

        // Gdy użytkownik zamknie strumień wejściowy (Ctrl+D)
        kill(pid, SIGKILL);
        wait(NULL);
    }

    mq_close(my_mq);
    mq_close(server_mq);
    mq_unlink(my_queue_name);

    return 0;
}