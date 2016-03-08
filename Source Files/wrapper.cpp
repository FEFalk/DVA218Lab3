
#include "shared.h"



/*
    Generic checksum calculation function
*/
unsigned short crc16(const unsigned char* data_p, int length)
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

void serialize(rtp* msgPacket, char *data)
{
    int *q = (int*)data;
    *q = msgPacket->flags;       q++;
    *q = msgPacket->id;   q++;
    *q = msgPacket->seq;     q++;
    *q = msgPacket->windowsize;     q++;
    *q = msgPacket->crc;     q++;

    char *p = (char*)q;
    int i = 0;
    while (i < strlen(msgPacket->data)+1)
    {
        *p = msgPacket->data[i];
        p++;
        i++;
    }
    q=NULL;
    p=NULL;
}
bool randomizePacket(rtp *sendPacket) {
    int r = rand() % 10;

    switch (r)
    {
        //Corrupted packet
        case 0:
        {
            sendPacket->crc=1;
        }
            break;
            //Packet out of order
        case 1:
            sendPacket->seq-=1;
            break;
            //Packet lost
        case 2:
            return false;
        default:
            break;
    }
    return true;
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
    sendPacket->id=uniqueIdentifier;
    char *serializedData = (char *)malloc((sizeof(rtp)+strlen(sendPacket->data)+1));

    rtp recvPacket;
    int nbytes;

    int LAR;
    while(1)
    {

        for(int i=0;totalActivePackets<windowSize && (currentSequenceNum+i) <= packetSize-1; i++)
        {
            sendPacket->seq=currentSequenceNum+i;
            if(currentSequenceNum+i==(packetSize-1))
                sendPacket->flags=LAST_DATA;
            else
                sendPacket->flags=DATA;

            sendPacket->crc=0;
            serialize(sendPacket, serializedData);
            sendPacket->crc = crc16((unsigned char *)serializedData, (strlen(serializedData)+1));


            usleep(1000*500);
            //cout << "Sending DATA-package " << sendPacket->seq << "." << endl;
            if(randomizePacket(sendPacket))
            {
                //Serializing data
                serialize(sendPacket, serializedData);
                nbytes = sendto(s, serializedData, sizeof(rtp)+strlen(sendPacket->data)+1, 0, (struct sockaddr*) &si_server, slen);
            }

            totalActivePackets++;
        }

        //If timed out
        if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
        {
            totalActivePackets=0;
            cout << "Timeout reached. Resending from sequence number " << currentSequenceNum << "." << endl;
            continue;
        }

        if(recvPacket.flags==ACK)
        {
            cout << "Received ACK " << recvPacket.seq << "." << endl;
            LAR=recvPacket.seq;

            //Setting short timeout and reading through queue for largest ACK received from server
            struct timeval timeout={0, 1000};
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout, sizeof(timeout));
            while(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) >= 0)
            {
                if(recvPacket.flags==ACK )
                {
                    //cout << "Received ACK " << recvPacket.seq << "." << endl;
                    if(recvPacket.seq > LAR)
                        LAR=recvPacket.seq;
                }
            }
            timeout={2, 0};
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout, sizeof(timeout));

            cout << "Largest ACK received is " << LAR << "." << endl;

            //Check sequence number etc.
            if(LAR>=currentSequenceNum)
            {
                if(LAR>=packetSize-1)
                {
                    cout << "Transfer is complete! " << endl;
                    break;
                }

                totalActivePackets-=((LAR+1)-currentSequenceNum);
                currentSequenceNum=LAR+1;
                cout << "Moving sequence number to " << currentSequenceNum << "." << endl;


            }
            else
            {
                totalActivePackets=0;
                currentSequenceNum=LAR+1;
                cout << "ACK " << LAR << " Has a lower sequence number than expected. Packet(s) lost.\n Resending from package " << currentSequenceNum << "..." << endl;
            }
        }
    }

    free(serializedData);
    free(sendPacket);
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
        if(randomizePacket(sendPacket))
            sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

        //If timed out
        if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
        {
            if(sendPacket->flags==FIN)
            {
                cout << "Timeout reached. Resending connection-teardown request (FIN)." << endl;
                continue;
            }
        }

        if(recvPacket.flags==FIN_ACK)
        {
            sendPacket->flags=ACK;

            cout << "Received FIN+ACK from server." << endl;
            while(1)
            {
                if(randomizePacket(sendPacket))
                    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

                cout << "Waiting to close connection..." << endl;
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
                {
                    connected=false;
                    break;
                }
                else if(recvPacket.flags==FIN_ACK)
                    cout << "Closing of connection was interrupted by FIN+ACK packet from server. Resending ACK..." << endl;
                else
                {
                    free(sendPacket);
                    return false;
                }

            }
        }
    }
    free(sendPacket);
    return true;
}

void terminateProgram(int s, int uniqueIdentifier, struct sockaddr_in si_server)
{
    if(!closeConnectionTo(s, uniqueIdentifier, si_server))
        cout << "Received bad package." << endl;
    else
        cout << "Connection shut down properly." << endl;

    cout << "Closing socket..." << endl;
    close(s);

    cout << "Terminating application..." << endl;
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
        if(randomizePacket(sendPacket))
            sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

        //If timed out
        if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
        {
            if(sendPacket->flags==SYN)
            {
                cout << "Timeout reached. Resending connection request (SYN)." << "." << endl;
                continue;
            }
        }

        if(recvPacket.flags==SYN_ACK)
        {
            *windowSize=recvPacket.windowsize;
            sendPacket->flags=ACK;

            cout << "Received SYN+ACK from server." << endl;
            while(1)
            {
                if(randomizePacket(sendPacket))
                    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

                cout << "Waiting to connect..." << endl;
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, &slen) < 0)
                {
                    connected=true;
                    break;
                }
                else if(recvPacket.flags==SYN_ACK)
                    cout << "Connection was interrupted by SYN+ACK packet from server. Resending ACK..." << endl;
                else
                {
                    free(sendPacket);
                    free(sendPacket->data);
                    return -1;
                }

            }
        }
    }
    free(sendPacket->data);
    free(sendPacket);
    return recvPacket.id;
}