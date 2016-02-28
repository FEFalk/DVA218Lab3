
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "shared.h"

#define SERVER_IP "127.0.0.1"

int main(void)
{
    struct sockaddr_in si_server, si_other;
    int s, i;
    socklen_t slen=sizeof si_server;
    int uniqueIdentifier, windowSize;
    char *errorMessage;
    int nbytes;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    sendPacket->data = (char *)calloc(sizeof(char), 256);
    rtp recvPacket;


    strcpy(sendPacket->data, "Hello 1 :^)");
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
        strcpy(errorMessage, "Failed to create socket");
        diep(errorMessage);
    }


    memset((char *) &si_server, 0, sizeof(si_server));

    si_server.sin_family = AF_INET;
    si_server.sin_addr.s_addr = inet_addr(SERVER_IP);
    si_server.sin_port = htons(PORT);

    if (inet_aton(SERVER_IP, &si_server.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


    sendPacket->flags=SYN;
    sendPacket->id=0;
    nbytes = sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

    printf("%d\n", nbytes);

    char buffer[256];

    recvfrom(s, &recvPacket, 256, 0, (struct sockaddr*) &si_other, &slen);

    if(recvPacket.flags==SYN_ACK)
    {
        uniqueIdentifier=recvPacket.id;
        windowSize=recvPacket.windowsize;

        sendPacket->flags=ACK;
        nbytes = sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);
    }

    printf("%d\n", nbytes);
    //TODO
    /*If timed out, proceed*/
    //recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, (socklen_t *)&slen);
    //if(recvPacket.flags==ACK)
    //{
    //    printf("Connected to the server!");
    //}

    sendPacket->flags=DATA;
    printf("Sending packet 1\n");
    strcpy(sendPacket->data, "Hello 1 :^)");
    //Change size to something more logical
    nbytes = sendto(s, (void *)sendPacket, 256, 0, (struct sockaddr*) &si_server, slen);

    printf("%d\n", nbytes);
    for (i=2; i<NPACK; i++) {
        recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, (socklen_t *)&slen);
        if(recvPacket.flags==ACK)
        {
            printf("Sending packet %d\n", i);
            fflush(stdout);
            sprintf(sendPacket->data, "Hello %d :^)", i);
            if((sendto(s, (void*) sendPacket, 256, 0, (struct sockaddr*) &si_server, slen)) == -1)
            {
                strcpy(errorMessage, "Failed the 'sendto()'");
                diep(errorMessage);
            }
            printf("%s", errorMessage);
        }
    }

    close(s);
    return 0;
}