
#ifndef CLIENT_WRAPPER_H
#define CLIENT_WRAPPER_H

unsigned short crc16(const unsigned char* data_p, int length);
void sendDataTo(int s, int uniqueIdentifier, char *msg, int windowSize, int packetSize, struct sockaddr_in si_server);
int connectTo(int s, int *windowSize, struct sockaddr_in si_server);
bool closeConnectionTo(int s, int uniqueIdentifier, struct sockaddr_in si_server);
void terminateProgram(int s, int uniqueIdentifier, struct sockaddr_in si_server);
bool randomizePacket(rtp *sendPacket);

#endif //CLIENT_WRAPPER_H
