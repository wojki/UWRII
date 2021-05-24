#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define SIZE_OF_BUFFOR 500

typedef struct datagram{
    size_t start;
    size_t len;
    bool recieved;
    int sockfd;
    struct sockaddr_in * ip_addr;
    uint8_t data[1000];
}Datagram;

typedef struct circular_buffer{
    Datagram tab[SIZE_OF_BUFFOR];
    int writeIndex;
    int readIndex;
    bool full;
}window;

int size_CB(window *buffer);
bool if_CB_full(window *buffer);
bool if_CB_empty(window *buffer);
Datagram* write_CB2(window *buffer, int start, int len, int sockfd, struct sockaddr_in* ip_address);
void write_CB(window *buffer, Datagram *elem);
void lookout_CB(window *buffer, Datagram *elem);
Datagram* read_CB(window *buffer);