
#ifndef CLIENT_WRAPPER_H
#define CLIENT_WRAPPER_H

void sendDataTo(int s, char *msg, int windowsize, int packetSize, struct sockaddr_in si_server);
int connectTo(int s, int *windowSize, struct sockaddr_in si_server);
void terminateProgram(int s);

#endif //CLIENT_WRAPPER_H
