#include "Cbuffer.h"

int size_CB(window *buffer)
{
    //printf("w=%d, r=%d\n",buffer->writeIndex,buffer->readIndex);
    return abs(buffer->writeIndex - buffer->readIndex);
}

bool if_CB_full(window *buffer)
{
    return buffer->full;
}

bool if_CB_empty(window *buffer)
{
    return ( (buffer->writeIndex == buffer->readIndex) && !buffer->full );
}

Datagram* write_CB2(window *buffer, int start, int len, int sockfd, struct sockaddr_in* ip_address)
{
    //Datagram dat;
    //printf("he?");
    //printf("iqin %d\n",(d->size_of_requested < d->size_of_file && size_CB(&d->current_window) < d->size_of_window));
    buffer->tab[buffer->writeIndex].start = start;
    buffer->tab[buffer->writeIndex].len = len;
    buffer->tab[buffer->writeIndex].recieved = false;
    buffer->tab[buffer->writeIndex].sockfd = sockfd;
    buffer->tab[buffer->writeIndex].ip_addr = ip_address;

    return &buffer->tab[buffer->writeIndex];
}

void write_CB(window *buffer, Datagram *elem)
{
    //printf("wCB1");
    buffer->tab[buffer->writeIndex] = *elem;
    //printf("wCB1");
    buffer->writeIndex++;
    //printf("wCB1");
    buffer->writeIndex %= SIZE_OF_BUFFOR;
    //printf("wCB1");
    if(buffer->writeIndex == buffer->readIndex)
        buffer->full = true;
}

void lookout_CB(window *buffer, Datagram *elem)
{
    *elem = buffer->tab[buffer->readIndex];
}

Datagram* read_CB(window *buffer)
{
    //printf("cokolwiek");
    //printf("readCB: %d, wI=%d, f? = %d\n",buffer->readIndex,buffer->writeIndex,if_CB_full(buffer));
    //if(if_CB_full(buffer))
    //    return;
    //printf("wI = %d",buffer->writeIndex);
    int temp = buffer->readIndex;
    //printf("po odczy");
    buffer->readIndex++;
    buffer->readIndex %= SIZE_OF_BUFFOR;
    buffer->full = false;
    return &buffer->tab[temp];
}