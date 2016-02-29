
#include "shared.h"

void terminateProgram(int s)
{
    cout << "Terminating connection...\n";
    close(s);

    cout << "Terminating application...\n";
    return;
}

void sendDataTo(int s, char *msg, int windowsize, int packetSize, struct sockaddr_in si_server)
{
    struct sockaddr_in si_other;
    socklen_t slen = sizeof si_server;
    int totalActivePackets=0;
    int currentSequenceNum=0;

    //TIMEOUT INITIALIZED
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

    //Initialize packet to send
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    sendPacket->data = (char *)calloc(sizeof(char), 256);
    strcpy(sendPacket->data, msg);
    sendPacket->flags=DATA;
    sendPacket->seq=0;
    rtp recvPacket;

    while(1)
    {
        for(int i=0;totalActivePackets<windowsize; i++)
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
int connectTo(int s, int *windowSize, struct sockaddr_in si_server)
{
    struct sockaddr_in si_other;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    sendPacket->data = (char *)calloc(sizeof(char), 256);
    rtp recvPacket;
    socklen_t slen=sizeof si_server;

    //TIMEOUT INITIALIZED
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

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
                else{
                    cout << "Received bad package. Terminating...\n";
                    terminateProgram(s);
                    return -1;
                }
            }
        }
    }
    return recvPacket.id;
}
