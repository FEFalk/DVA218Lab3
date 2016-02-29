//
// Created by frenning on 2016-02-29.
//


#include "shared.h"

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

void connectTo(int s, struct sockaddr_in si_client, int *uniqueIdentifier)
{
    struct sockaddr_in si_other;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    rtp *recvPacket = (rtp *)calloc(sizeof(rtp), 1);
    socklen_t slen=sizeof si_client;

    bool connected=false;
    while(!connected)
    {
        recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen);
        if(recvPacket->flags==SYN)
        {
            sendPacket->flags=SYN_ACK;
            sendPacket->id=++*uniqueIdentifier;
            cout << "Received SYN from client.\n";
            while(1)
            {
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
                cout << "Waiting to connect...\n";
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
                {
                    if(sendPacket->flags==SYN_ACK)
                    {
                        cout << "Timeout reached. Resending connection request (SYN)." << ".\n";
                        continue;
                    }
                }
                else if(recvPacket->flags==SYN) {
                    cout << "Connection was interrupted by SYN+ACK packet from server. Resending ACK...\n";
                    continue;
                }
                else if(recvPacket->flags == ACK)
                {
                    connected=true;
                    break;
                }
                else
                    continue;
            }
        }

    }

}
