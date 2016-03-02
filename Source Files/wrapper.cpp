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

void deserialize(char *data, rtp* msgPacket)
{
    int *q = (int*)data;
    msgPacket->flags = *q;       q++;
    msgPacket->id = *q;   q++;
    msgPacket->seq = *q;     q++;
    msgPacket->windowsize = *q;     q++;
    msgPacket->crc = *q;     q++;

    char *p = (char*)q;
    int i = 0;
    while (i < 256)
    {
        msgPacket->data[i] = *p;
        p++;
        i++;
    }
}

void terminateProgram(int s)
{
    cout << "Terminating connection...\n";
    close(s);

    cout << "Terminating application...\n";
    return;
}

void closeTransmission(char **acceptedPackets, rtp *sendPacket, char *serializedData)
{
    for(int i=0;i<BUFLEN;i++)
    {
        if(acceptedPackets[i]!=NULL)
            free(acceptedPackets[i]);
    }
    free(acceptedPackets);
    free(sendPacket);
    free(serializedData);
}

int recvDataFrom(int s, rtp *recvPacket, struct sockaddr_in si_client)
{
    //If the first received packet is out of sync -> exit.
    if(recvPacket->seq!=0)
        return -1;


    struct sockaddr_in si_other;
    socklen_t slen = sizeof si_client;
    char **acceptedPackets;
    acceptedPackets = (char **)calloc(sizeof(char *), BUFLEN);
    int currentSequenceNum=0;

    struct timeval timeout={0, 0};
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    cout << "Client is sending DATA-packets, Initializing transmission...\n";
    sendPacket->seq=currentSequenceNum++;
    sendPacket->flags=ACK;
    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);



    char* serializedData = (char *)malloc(BUFLEN);

    //CHECK SEQUENCE NUMBER STORE IN BUFF SEND ACK
    while(1)
    {
        if(recvPacket->seq==0 && recvPacket->flags==LAST_DATA)
        {
            currentSequenceNum=0;
        }
        else{
            cout << "Waiting for packets...\n";
            recvfrom(s, serializedData, BUFLEN, 0, (struct sockaddr*) &si_other, &slen);
            sendPacket->seq=currentSequenceNum;
        }
        deserialize(serializedData, (struct rtp_struct*)recvPacket);

        if(recvPacket->flags==DATA || recvPacket->flags==LAST_DATA)
        {
            //If received CORRECT frame
            if (recvPacket->seq == currentSequenceNum) {
                cout << "Received correct frame (" << recvPacket->seq << ") Sending ACK...\n";
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
                if (acceptedPackets[currentSequenceNum] == NULL) {
                    acceptedPackets[currentSequenceNum] = (char *) calloc(sizeof(char), strlen(recvPacket->data) + 1);
                    strcpy(acceptedPackets[currentSequenceNum], recvPacket->data);
                }
                else
                    cout << "Packet " << recvPacket->seq << " already received. Rejecting it and moving on...\n";

                if (recvPacket->flags == LAST_DATA) {
                    cout << "Last packet received (" << recvPacket->seq << ")! Waiting before closing transmission...\n";

                    bool receivedData=false;

                    //Wait for any additional packets from client before closing transmission.
                    while(1)
                    {
                        //Setting 2 second timeout-value
                        timeout={2, 0};
                        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout, sizeof(timeout));

                        //If timed out, close transmission
                        if (recvfrom(s, serializedData, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) < 0) {
                            cout << "Transmission complete! Closing transmission...\n";
                            closeTransmission(acceptedPackets, sendPacket, serializedData);
                            return 0;
                        }
                        else {
                            //Read first message
                            deserialize(serializedData, (struct rtp_struct *) recvPacket);
                            if (recvPacket->flags == DATA)
                                receivedData = true;

                            //If there are messages stuck in the queue, we check this by setting a small
                            // timeout-value and loop recvfrom until we timeout/no messages are left.
                            timeout = {0, 1000};
                            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
                            while (recvfrom(s, serializedData, sizeof(rtp), 0, (struct sockaddr *) &si_other, &slen) >= 0) {
                                deserialize(serializedData, (struct rtp_struct *) recvPacket);
                                if (recvPacket->flags == DATA)
                                    receivedData = true;
                            }
                            //If we received any additional DATA-packets that's in our window-size, we resend the last ACK.
                            // If bad packet-flag/sequence number was received we discard it and close the transmission.
                            if (receivedData && recvPacket->seq > (currentSequenceNum-WINDOWSIZE)) {
                                cout << "Received additional DATA-packet(s). Sending last ACK again...";
                                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
                            }
                            else
                            {
                                cout << "Received bad package. Assuming transmission is complete.\nClosing transmission...\n";
                                closeTransmission(acceptedPackets, sendPacket, serializedData);
                                return 0;
                            }

                        }
                    }
                }
                //cout << "Packet data: " << recvPacket->data;
                currentSequenceNum++;
            }

                //If received frame larger than expected
            else if (recvPacket->seq > currentSequenceNum) {
                if (acceptedPackets[recvPacket->seq] == NULL) {
                    acceptedPackets[recvPacket->seq] = (char *) calloc(sizeof(char), strlen(recvPacket->data) + 1);
                    strcpy(acceptedPackets[recvPacket->seq], recvPacket->data);
                }
                else
                    cout << "Packet " << recvPacket->seq << " already received. Rejecting it and moving on...\n";

                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
            }

                //If frame is corrupted
            else if ((crc16((unsigned char *) recvPacket->data, (unsigned char) strlen(recvPacket->data))) !=
                     recvPacket->crc) {
                cout << "Packet data from frame " << recvPacket->seq << " is corrupted! Ignoring it...";
            }

                //If frame is smaller than expected
            else {
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
            }
        }
    }
}

sockaddr_in connectTo(int s, int *uniqueIdentifier)
{
    struct sockaddr_in si_client;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    rtp recvPacket;
    socklen_t slen=sizeof si_client;

    bool connected=false;
    while(!connected)
    {
        recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_client, &slen);
        if(recvPacket.flags==SYN)
        {
            sendPacket->flags=SYN_ACK;
            sendPacket->windowsize=WINDOWSIZE;
            sendPacket->id=++(*uniqueIdentifier);
            cout << "Received SYN from client.\n";
            while(1)
            {
                sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
                cout << "Waiting to connect...\n";
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_client, &slen) < 0)
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
                    cout << "Received ACK from client. The client is now connected!\n";
                    connected=true;
                    break;
                }
            }
        }
    }
    free(sendPacket);
    return si_client;
}

bool closeConnectionFrom(int s, int uniqueIdentifier, struct sockaddr_in si_server)
{

}
