
#include "shared.h"

/**
    Datacommunication, DVA218 - Lab 3
    wrapper.cpp
    Purpose: Contains functions for server.cpp

    @author Filiph Eriksson-Falk - ffk13001, Fredrik Frenning - ffg12002
    @date 08/03/2016
*/



/*
    crc16 - Generic checksum calculation function

    @param data_p - The string to be used for calculating the checksum
    @param length - The length of the string data_p
    @return - The calculated checksum value
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

/*
    serialize - Function for serializing a rtp struct

    @param msgPacket - The struct to be used for serializing.
    @param data - The included message(string) of the packet.
*/
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

/*
    deserialize - Function to deserialize a rtp struct that has been serialized

    @param data - The serialized struct.
    @param msgPacket - Pointer to the rtp which is going to be receiving the deserialized packet.
*/
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

/*
    terminateProgram - Function to close the server's socket.

    @param s - The server's socket-id.
*/
void terminateProgram(int s)
{
    cout << "Terminating connection..." << endl;
    close(s);

    cout << "Terminating application..." << endl;
    return;
}

/*
    randomizePacket - Randomizes an error for a packet.
    Choosing between Corrupted packet, Packet out of order and Lost packet.

    @param sendPacket - The packet to receive a random error.

    @return - true or false depending on if the packet should be lost or not.
*/
bool randomizePacket(struct rtp_struct *sendPacket) {
    int r = rand() % 10;

    switch (r)
    {
        //Corrupted packet
        case 0:
            sendPacket->crc=155;
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

/*
    closeTransmission - Frees allocated memory from the "recvDataFrom" function

    @param acceptedPackets - Array of strings that have been given allocated memory.
    @param sendPacket - Packet-struct to send to the server.
    @param serializedData - Pointer to serialized rtp-struct.
*/
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

/*
    recvDataFrom - Sliding Window protocol.

    @param s - The server's socket.
    @param recvPacket - The initial packet received from the client.
    @param si_client - The client's information such as IP-address and such.

    @return - Error values.
*/
int recvDataFrom(int s, rtp *recvPacket, struct sockaddr_in si_client)
{
    //If the first received packet is out of sync -> exit.
    if(recvPacket->seq!=0)
        return -1;

    bool isFirstPacket=true;
    struct sockaddr_in si_other;
    socklen_t slen = sizeof si_client;
    char **acceptedPackets;
    acceptedPackets = (char **)calloc(sizeof(char *), BUFLEN);
    int currentSequenceNum=0;
    int tempCRC;
    struct timeval timeout={0, 0};
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));


    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    cout << "Client is sending DATA-packets, Initializing transmission..." << endl;
    sendPacket->seq=currentSequenceNum;
    sendPacket->flags=ACK;

    char* serializedData = (char *)malloc(BUFLEN);
    serialize(recvPacket, serializedData);

    //CHECK SEQUENCE NUMBER STORE IN BUFF SEND ACK
    while(1)
    {
        if(isFirstPacket)
            isFirstPacket=false;
        else{
            cout << "Waiting for packets..." << endl;
            recvfrom(s, serializedData, BUFLEN, 0, (struct sockaddr*) &si_other, &slen);
            sendPacket->seq=currentSequenceNum;
            deserialize(serializedData, (struct rtp_struct*)recvPacket);
        }

        if(recvPacket->flags==DATA || recvPacket->flags==LAST_DATA)
        {
            tempCRC=recvPacket->crc;
            recvPacket->crc=0;
            //If frame is corrupted
            if ((crc16((unsigned char *) serializedData, (strlen(serializedData)+1))) != tempCRC) {
                cout << "Packet data from frame " << recvPacket->seq << " is corrupted! Ignoring it..." << endl;
            }
            //If received CORRECT frame
            else if (recvPacket->seq == currentSequenceNum) {
                cout << "Received correct frame (" << recvPacket->seq << ") Sending ACK " << currentSequenceNum << "..." << endl;
                if(randomizePacket(sendPacket))
                    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
                if (acceptedPackets[currentSequenceNum] == NULL) {
                    acceptedPackets[currentSequenceNum] = (char *) calloc(sizeof(char), strlen(recvPacket->data) + 1);
                    strcpy(acceptedPackets[currentSequenceNum], recvPacket->data);
                }
                else
                    cout << "Packet " << recvPacket->seq << " already received. Rejecting it and moving on..." << endl;

                if (recvPacket->flags == LAST_DATA) {
                    cout << "Last packet received (" << recvPacket->seq << ")! Waiting before closing transmission..." << endl;

                    bool receivedData=false;

                    //Wait for any additional packets from client before closing transmission.
                    while(1)
                    {
                        //Setting 2 second timeout-value
                        timeout={2, 0};
                        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout, sizeof(timeout));

                        //If timed out, close transmission
                        if (recvfrom(s, serializedData, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) < 0) {
                            cout << "Transmission complete! Closing transmission..." << endl;
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
                                if(randomizePacket(sendPacket))
                                    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
                            }
                            else
                            {
                                cout << "Received bad package. Assuming transmission is complete.\nClosing transmission..." << endl;
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
                cout << "Packet " << recvPacket->seq << " is larger than expected. (Expected " << currentSequenceNum << endl;
                sendPacket->seq=currentSequenceNum-1;
                cout << "Sending ACK " << sendPacket->seq << "..." << endl;
                if (acceptedPackets[recvPacket->seq] == NULL) {
                    acceptedPackets[recvPacket->seq] = (char *) calloc(sizeof(char), strlen(recvPacket->data) + 1);
                    strcpy(acceptedPackets[recvPacket->seq], recvPacket->data);
                }
                else
                    cout << "Packet " << recvPacket->seq << " already received. Rejecting it and moving on..." << endl;
                if(randomizePacket(sendPacket))
                    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
            }

                //If frame is smaller than expected
            else {
                sendPacket->seq=currentSequenceNum-1;
                cout << "Packet " << recvPacket->seq << " received is smaller than expected (" << currentSequenceNum <<
                        ").\nSending ACK " << sendPacket->seq << " again..." << endl;
                if(randomizePacket(sendPacket))
                    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr *) &si_client, slen);
            }
        }
    }
}

/*
    connectTo - Connection Setup

    @param s - The server's socket.
    @param uniqueIdentifier - Pointer to the unique identifier of the client's socket.

    @return - The connected client's socket information.
*/
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
            cout << "Received SYN from client." << endl;
            while(1)
            {
                cout << "Sending SYN+ACK-packet to client..." << endl;
                if(randomizePacket(sendPacket))
                    sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
                cout << "Waiting for ACK-packet from client..." << endl;
                if(recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_client, &slen) < 0)
                {
                    cout << "Timeout reached. Resending (SYN+ACK)." << endl;
                    continue;
                }
                else if(recvPacket.flags==SYN) {
                    cout << "Connection was interrupted by SYN packet from client. Resending SYN+ACK..." << endl;
                    continue;
                }
                else if(recvPacket.flags == ACK)
                {
                    cout << "Received ACK from client. The client is now connected!" << endl;
                    connected=true;
                    break;
                }
            }
        }
    }
    free(sendPacket);
    return si_client;
}

/*
    connectTo - Connection Teardown

    @param s - The server's socket.
    @param recvPacket - Pointer to the initial packet received from the client.
    @param si_client - The client's information such as IP-address and such.

    @return - true of false depending on if the connection got closed correctly.
*/
bool closeConnectionFrom(int s, rtp *recvPacket, struct sockaddr_in si_client)
{
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    socklen_t slen=sizeof si_client;

    sendPacket->flags=FIN_ACK;
    cout << "Received FIN from client." << endl;
    while(1)
    {
        if(randomizePacket(sendPacket))
            sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_client, slen);
        cout << "Waiting to close..." << endl;
        if(recvfrom(s, recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_client, &slen) < 0)
        {
            cout << "Timeout reached. Resending (FIN+ACK)." << endl;
            continue;
        }
        else if(recvPacket->flags==FIN) {
            cout << "Connection was interrupted by FIN packet from client. Resending FIN+ACK..." << endl;
            continue;
        }
        else if(recvPacket->flags == ACK)
        {
            cout << "Received ACK from client. Closing connection..." << endl;
            break;
        }
        else{
            cout << "Received something weird. Aborting connection-teardown..." << endl;
            free(sendPacket);
            return false;
        }
    }
    free(sendPacket);
    return true;
}