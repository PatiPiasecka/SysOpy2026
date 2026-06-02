#ifndef CHAT_H
#define CHAT_H

#define SERVER_QUEUE "/chat_server_queue"
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 512

// Typy komunikatów (operacji)
typedef enum {
    INIT,   //Pierwsze połączenie klienta
    TEXT,   //Wiadomość tekstowa do rozesłania
    STOP    // Wyrejestrowanie klienta przy wyjściu
} MsgType;

// Struktura przesyłana wewnątrz kolejki (komunikat)
typedef struct {
    MsgType type;
    int client_id;
    char client_queue_name[64]; // Przekazywane przy INIT
    char text[MAX_MSG_SIZE];    // Treść wiadomości tekstowej
} Message;

#endif