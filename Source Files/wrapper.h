/*
    Datacommunication, DVA218 - Lab 3
    wrapper.h
    Purpose: Declarations for functions in wrapper.cpp

    @author Filiph Eriksson-Falk - ffk13001, Fredrik Frenning - ffg12002
    @date 08/03/2016
*/

#ifndef SERVER_WRAPPER_H
#define SERVER_WRAPPER_H
#include "shared.h"

unsigned short crc16(const unsigned char* data_p, unsigned char length);
void deserialize(char *data, struct rtp_struct* msgPacket);
bool randomizePacket(struct rtp_struct *sendPacket);
int recvDataFrom(int s, struct rtp_struct *recvPacket, struct sockaddr_in si_client);
sockaddr_in connectTo(int s, int *uniqueIdentifier);
void terminateProgram(int s);
void closeTransmission(char **acceptedPackets, struct rtp_struct *sendPacket, char *serializedData);
bool closeConnectionFrom(int s, struct rtp_struct *recvPacket, struct sockaddr_in si_client);

#endif //SERVER_WRAPPER_H
