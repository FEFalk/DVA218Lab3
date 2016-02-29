//
// Created by frenning on 2016-02-29.
//


#include "shared.h"
/*
    Generic checksum calculation function
*/
unsigned short crc16(const unsigned char* data_p, unsigned char length)
{
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}

void terminateProgram(int s)
{
    cout << "Terminating connection...\n";
    close(s);

    cout << "Terminating application...\n";
    return;
}

void recvDataFrom(int s, char *msg, int windowsize, int packetSize, struct sockaddr_in si_client)
{
    struct sockaddr_in si_other;
    socklen_t slen = sizeof si_client;
    int totalActivePackets=0;
    int currentSequenceNum=0;
    int buff[256];
    //Initialize packet to send
    rtp *recvPacket = (rtp *)calloc(sizeof(rtp), 1);
    recvPacket->data = (char *)calloc(sizeof(char), 256);
    strcpy(recvPacket->data, msg);


    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);

    //CHECK SEQUENCE NUMBER STORE IN BUFF SEND ACK

    while(1)
    {
        for(int i=0;totalActivePackets<windowsize; i++)
        {
            if(recvPacket->seq == currentSequenceNum){
                printf("%s", recvPacket->data);
                sendPacket->flags=ACK;
                sendPacket->seq=buff[currentSequenceNum];
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
                currentSequenceNum++;
            }
            else if(recvPacket->seq > currentSequenceNum)
            {
                buff[currentSequenceNum] = recvPacket->seq;
                sendPacket->flags=ACK;
                sendPacket->seq=currentSequenceNum;
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
            }
            else
            {
                sendPacket->flags=ACK;
                sendPacket->seq=currentSequenceNum;
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
            }

            totalActivePackets++;
        }
        break;
    }
}
