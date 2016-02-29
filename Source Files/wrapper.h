//
// Created by frenning on 2016-02-29.
//

#ifndef SERVER_WRAPPER_H
#define SERVER_WRAPPER_H

unsigned short crc16(const unsigned char* data_p, unsigned char length);
void recvDataFrom(int s, char *msg, int windowsize, int packetSize, struct sockaddr_in si_client);
void connectTo(int s, struct sockaddr_in si_client, int *uniqueIdentifier);
void terminateProgram(int s);

#endif //SERVER_WRAPPER_H
