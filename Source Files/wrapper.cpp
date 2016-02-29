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

int recvDataFrom(int s, rtp *recvPacket, struct sockaddr_in si_client)
{
    struct sockaddr_in si_other;
    socklen_t slen = sizeof si_client;
    char *acceptedPackets[BUFLEN];
    int currentSequenceNum=0;

    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);

    //If the first received packet is out of sync -> exit.
    if(recvPacket->seq!=0)
        return -1;

    cout << "Client is sending DATA-packets, Initializing transmission...\n";
    sendPacket->seq=currentSequenceNum++;
    sendPacket->flags=ACK;
    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);

    //CHECK SEQUENCE NUMBER STORE IN BUFF SEND ACK
    while(1)
    {
        cout << "Waiting for packets...\n";
        recvfrom(s, recvPacket, BUFLEN, 0, (struct sockaddr*) &si_other, &slen);
        sendPacket->seq=currentSequenceNum;

        //If received CORRECT frame
        if(recvPacket->seq == currentSequenceNum)
        {
            cout << "Received correct frame (" << recvPacket->seq << ").\n";
            if(acceptedPackets[currentSequenceNum]==0)
                strcpy(acceptedPackets[currentSequenceNum], recvPacket->data);
            else
                cout << "Packet " << recvPacket->seq << " already received. Rejecting it and moving on...\n";

            if(recvPacket->flags==LAST_DATA)
            {
                cout << "Last packet received! " << recvPacket->seq << " Closing transmission...\n";
                break;
            }
            //cout << "Packet data: " << recvPacket->data;
            sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
            currentSequenceNum++;
        }

            //If received frame larger than expected
        else if(recvPacket->seq > currentSequenceNum)
        {
            strcpy(acceptedPackets[recvPacket->seq], recvPacket->data);
            sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
        }

            //If frame is corrupted
        else if(crc16((unsigned char *)recvPacket->data, (unsigned char)strlen(recvPacket->data)) != recvPacket->crc)
        {
            cout << "Packet data from frame " << recvPacket->seq << " is corrupted! Ignoring it...";
        }

            //If frame is smaller than expected
        else
        {
            sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
        }
    }
}

void connectTo(int s, struct sockaddr_in si_client, int *uniqueIdentifier)
{
    struct sockaddr_in si_other;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    rtp recvPacket;
    socklen_t slen=sizeof si_client;

    bool connected=false;
    while(!connected)
    {
        recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen);
        if(recvPacket.flags==SYN)
        {
            sendPacket->flags=SYN_ACK;
            sendPacket->id=++(*uniqueIdentifier);
            cout << "Received SYN from client.\n";
            while(1)
            {
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
                cout << "Waiting to connect...\n";
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
                {
                    cout << "Timeout reached. Resending (SYN+ACK)." << ".\n";
                    continue;
                }
                else if(recvPacket.flags==SYN) {
                    cout << "Connection was interrupted by SYN packet from client. Resending SYN+ACK...\n";
                    continue;
                }
                else if(recvPacket.flags == ACK)
                {
                    connected=true;
                    break;
                }
            }
        }
    }
}

bool closeConnectionFrom(int s, int uniqueIdentifier, struct sockaddr_in si_server)
{

}
