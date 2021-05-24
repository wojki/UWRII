#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>

#include "Cbuffer.h"

void send_get_request(Datagram *d,struct sockaddr *ip_addr){
    char message[30];
    sprintf(message, "GET %zu %zu\n", d->start, d->len);
    int message_len = strlen(message);
    if (sendto(d->sockfd, message, strlen(message), 0, (struct sockaddr *)ip_addr, sizeof(*ip_addr)) != message_len)
    {
        printf("Sending unsuccesseful\nmes_len==%d\nmes==%s\n",d->sockfd,message);
        exit(EXIT_FAILURE);
    }
};

typedef struct downloader{
    size_t size_of_file, size_of_window, port, size_of_downloaded, size_of_requested, progress;
    int sockfd;
    const char * ip_addr;
    struct sockaddr_in ip_address;
    FILE *out;
    uint8_t buffer[IP_MAXPACKET+1];
    window current_window;
}Downloader;

int iter = 0;
void increase_window_if_needed(Downloader *d){
    if (d->size_of_requested < d->size_of_file && size_CB(&d->current_window) < d->size_of_window){
        size_t s = d->size_of_requested + 1000 < d->size_of_file ? 1000 : d->size_of_file - d->size_of_requested;

        printf("%d\n",iter++);
        /*Datagram dat;
        //printf("he?");
        //printf("iqin %d\n",(d->size_of_requested < d->size_of_file && size_CB(&d->current_window) < d->size_of_window));
        dat.start = d->size_of_requested;
        dat.len = s;
        dat.recieved = false;
        dat.sockfd = d->sockfd;
        dat.ip_addr = &d->ip_address;
        
        write_CB(&d->current_window,&dat);*/
        //lookout_CB(&d->current_window,&dat);
        //Datagram *dat_ptr = &dat;
        send_get_request(write_CB2(&d->current_window,d->size_of_requested,s,d->sockfd,&d->ip_addr)
                        ,(struct sockaddr *)&d->ip_address);
        d->size_of_requested += s;
    }
};

void receive_and_update(Downloader *d){
    struct sockaddr_in sender;
    socklen_t sock_len = sizeof(sender);
    ssize_t packet_len = recvfrom(d->sockfd, d->buffer, IP_MAXPACKET, MSG_DONTWAIT, (struct sockaddr *)&sender, &sock_len);
    if (packet_len < 0) {
        printf("Reciving unsuccessful");
        exit(EXIT_FAILURE);
	}
    size_t new_start,new_len;
    Datagram *datagram;
    if (d->ip_address.sin_addr.s_addr == sender.sin_addr.s_addr
    && d->ip_address.sin_port == sender.sin_port 
    && sscanf((char*)d->buffer, "DATA %zu %zu\n",&new_start, &new_len) == 2 ){
        int temp = size_CB(&d->current_window)-1;
        for (int j = 0 ; j < temp ; j++ )
        {
            printf("przed prfzeczytaniu %d %d\n",new_start,new_len);
            datagram = read_CB(&d->current_window);
            if (datagram->start == new_start && datagram->len == new_len && !datagram->recieved){
                printf("sprawdzenie pakietu\n");
                char message[30];
                sprintf(message, "GET %zu %zu\n", new_start, new_len);
                ssize_t message_len = strlen(message) + 1;
                char* new_data = (char*)d->buffer + message_len;
                for (size_t i=0;i<new_len;i++){
                    datagram->data[i] = new_data[i];
                }
                datagram->recieved = true;
                printf("losowy");
                break;
            }
        }
    }
    lookout_CB(&d->current_window,datagram);
    while (!if_CB_empty(&d->current_window) && datagram->recieved)
    {
        datagram = read_CB(&d->current_window);
        fwrite(datagram->data,sizeof(char),datagram->len,d->out);

        d->size_of_downloaded += datagram->len;
        increase_window_if_needed(d);
        if ((100*d->size_of_downloaded)/d->size_of_file > d->progress){
            d->progress = (100*d->size_of_downloaded)/d->size_of_file;
            printf("%zu%%\n",d->progress);
        }
    }
};

int main(int argc, char *argv[])
{
    size_t port,size;
    struct sockaddr_in sai;

    // sprawdzenie danych wejściowych
	if (argc != 5)
    {
        printf("Wrong number of arguments\n");
		exit(EXIT_FAILURE);
    }
    if (inet_pton(AF_INET, argv[1], &(sai.sin_addr)) != 1)
    {
        printf("Invalid Ip Addres: %s\n",argv[1]); 
		exit(EXIT_FAILURE);
    }
    if (sscanf(argv[2],"%zu",&port) != 1)
    {
        printf("Invalid second argument\n"); 
		exit(EXIT_FAILURE);
    }
    if (sscanf(argv[4],"%zu",&size) != 1)
    {
        printf("Invalid fourth argument\n"); 
		exit(EXIT_FAILURE);
    }

    Downloader downloader;
    downloader.size_of_file = size;
    downloader.port = port;
    downloader.ip_addr = argv[1];
    downloader.size_of_window = SIZE_OF_BUFFOR;
    downloader.progress = 0;
    downloader.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if  (downloader.sockfd < 0)
    {
        printf("Sockfd error");
        exit(EXIT_FAILURE);
    }
    memset(&downloader.ip_address, 0, sizeof(downloader.ip_address));
    downloader.ip_address.sin_family = AF_INET;
    downloader.ip_address.sin_port = htons(downloader.port);
    inet_pton(AF_INET, downloader.ip_addr, &downloader.ip_address.sin_addr);

    downloader.out = fopen(argv[3],"w");


    downloader.current_window.readIndex = 0;
    downloader.current_window.writeIndex = 0;
    downloader.current_window.full = false;
    for (size_t i=0;i<downloader.size_of_window;i++)
        increase_window_if_needed(&downloader);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 20000;
    for(;;){
        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(downloader.sockfd, &descriptors);
        int ready = select(downloader.sockfd + 1, &descriptors, NULL, NULL, &tv);
        if (ready < 0)
        {
            printf("Select error\n"); 
		    exit(EXIT_FAILURE);
        }
        else if (ready == 0)
        {
            for (size_t j = 0 ; j < size_CB(&downloader.current_window) ; j++ )
            {
                send_get_request(&downloader.current_window.tab[j],(struct sockaddr *)&downloader.ip_address);
            }
            tv.tv_sec = 0;
            tv.tv_usec = 20000;
        } 
        else
        {
            receive_and_update(&downloader);
            if (if_CB_empty(&downloader.current_window)){
                fclose(downloader.out);
                break;
            }
        }
    }

	return EXIT_SUCCESS;
}