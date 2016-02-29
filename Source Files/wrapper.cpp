
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


void sendDataTo(int s, int uniqueIdentifier, char *msg, int windowSize, int packetSize, struct sockaddr_in si_server)
{
    struct sockaddr_in si_other;
    socklen_t slen = sizeof si_server;
    int totalActivePackets=0;
    int currentSequenceNum=0;

    //Initialize packet to send
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    sendPacket->data = (char *)calloc(sizeof(char), 256);
    strcpy(sendPacket->data, msg);
    sendPacket->crc=0;
    sendPacket->crc = crc16((unsigned char *)msg, (unsigned char)strlen(msg));

    sendPacket->flags=DATA;
    sendPacket->id=uniqueIdentifier;
    sendPacket->seq=0;
    rtp recvPacket;

    while(1)
    {
        for(int i=0;totalActivePackets<windowSize; i++)
        {
            sendPacket->seq=currentSequenceNum+i;
            sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);
            totalActivePackets++;
        }

        //If timed out
        if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
        {
            totalActivePackets=0;
            cout << "Timeout reached. Resending from sequence number " << currentSequenceNum << ".\n";
            continue;
        }

        cout << "Received ACK " << recvPacket.seq << ".\n";

        //Check sequence number etc.
        if(recvPacket.seq>=currentSequenceNum)
        {
            totalActivePackets-=(recvPacket.seq+1)-currentSequenceNum;
            currentSequenceNum=recvPacket.seq+1;
            cout << "Moving sequence number to " << currentSequenceNum << ".\n";

            if(currentSequenceNum>=packetSize)
            {
                cout << "Transfer is complete! \n";
                break;
            }
        }
        else
        {
            totalActivePackets=0;
            currentSequenceNum=recvPacket.seq+1;
            cout << "Resending from package " << currentSequenceNum << "...\n";
        }
    }
}




bool closeConnectionTo(int s, int uniqueIdentifier, struct sockaddr_in si_server)
{
    struct sockaddr_in si_other;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    rtp recvPacket;
    socklen_t slen=sizeof si_server;

    sendPacket->flags=FIN;
    sendPacket->id=uniqueIdentifier;
    bool connected=true;
    while(connected)
    {
        sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

        //If timed out
        if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
        {
            if(sendPacket->flags==FIN)
            {
                cout << "Timeout reached. Resending connection request (FIN)." << ".\n";
                continue;
            }
        }

        if(recvPacket.flags==FIN_ACK)
        {
            sendPacket->flags=ACK;

            cout << "Received FIN+ACK from server.\n";
            while(1)
            {
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

                cout << "Waiting to close connection...\n";
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
                {
                    connected=false;
                    break;
                }
                else if(recvPacket.flags==FIN_ACK)
                    cout << "Closing of connection was interrupted by FIN+ACK packet from server. Resending ACK...\n";
                else
                    return false;
            }
        }
    }
    return true;
}

void terminateProgram(int s, int uniqueIdentifier, struct sockaddr_in si_server)
{
    if(!closeConnectionTo(s, uniqueIdentifier, si_server))
        cout << "Received bad package.\n";
    else
        cout << "Connection shut down properly.\n";

    cout << "Closing socket...\n";
    close(s);

    cout << "Terminating application...\n";
    return;
}

int connectTo(int s, int *windowSize, struct sockaddr_in si_server)
{
    struct sockaddr_in si_other;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    sendPacket->data = (char *)calloc(sizeof(char), 256);
    rtp recvPacket;
    socklen_t slen=sizeof si_server;

    sendPacket->flags=SYN;
    sendPacket->id=0;
    bool connected=false;
    while(!connected)
    {
        sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

        //If timed out
        if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
        {
            if(sendPacket->flags==SYN)
            {
                cout << "Timeout reached. Resending connection request (SYN)." << ".\n";
                continue;
            }
        }

        if(recvPacket.flags==SYN_ACK)
        {
            *windowSize=recvPacket.windowsize;
            sendPacket->flags=ACK;

            cout << "Received SYN+ACK from server.\n";
            while(1)
            {
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

                cout << "Waiting to connect...\n";
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
                {
                    connected=true;
                    break;
                }
                else if(recvPacket.flags==SYN_ACK)
                    cout << "Connection was interrupted by SYN+ACK packet from server. Resending ACK...\n";
                else
                    return -1;
            }
        }
    }
    return recvPacket.id;
}