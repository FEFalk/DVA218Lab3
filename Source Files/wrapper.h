
/*
    Datacommunication, DVA218 - Lab 3
    wrapper.h
    Purpose: Declarations for functions in wrapper.cpp

    @author Filiph Eriksson-Falk - ffk13001, Fredrik Frenning - ffg12002
    @date 08/03/2016
*/

#ifndef CLIENT_WRAPPER_H
#define CLIENT_WRAPPER_H

unsigned short crc16(const unsigned char* data_p, int length);
void sendDataTo(int s, int uniqueIdentifier, char *msg, int windowSize, int packetSize, struct sockaddr_in si_server);
int connectTo(int s, int *windowSize, struct sockaddr_in si_server);
bool closeConnectionTo(int s, int uniqueIdentifier, struct sockaddr_in si_server);
void terminateProgram(int s, int uniqueIdentifier, struct sockaddr_in si_server);
bool randomizePacket(rtp *sendPacket);

#endif //CLIENT_WRAPPER_H
