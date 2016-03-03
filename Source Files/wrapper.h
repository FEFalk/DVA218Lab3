//
// Created by frenning on 2016-02-29.
//

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
bool closeConnectionFrom(int s, int *uniqueIdentifier, struct sockaddr_in si_server);

#endif //SERVER_WRAPPER_H
