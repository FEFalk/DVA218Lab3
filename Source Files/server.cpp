#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "shared.h"


int main(void)
{
    setbuf(stdout, NULL);
	struct sockaddr_in si_me, si_other;
	int s, i;
	socklen_t slen = sizeof si_other;
	rtp sendPacket;
	rtp *recvPacket = (rtp *)calloc(sizeof(rtp), 1);
	recvPacket->data = (char *)calloc(sizeof(char), 256);
	char errorMessage[512];
	int uniqueIdentifier=0;
	int nbytes;
	printf("hej");
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		strcpy(errorMessage, "Failed to create socket");
		diep(errorMessage);
	}

	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (struct sockaddr*) &si_me, sizeof(si_me))==-1)
	{
		strcpy(errorMessage, "Failed to bind socket");
		diep(errorMessage);
	}

	while(1)
	{
		for (i=0; i<NPACK; i++) {
			if ((nbytes = recvfrom(s, recvPacket, 256, 0, (struct sockaddr*) &si_other, &slen))==-1)
			{
				strcpy(errorMessage, "Failed to recvfrom");
				diep(errorMessage);
			}
			printf("%d", nbytes);
			printf("hweghjg");
			switch(recvPacket->flags)
			{
				case DATA:
				{
					printf("Received message: %s\n", recvPacket->data);
					sendPacket.flags=ACK;
					sendto(s, (void *) &sendPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, slen);
				}
					break;
				case ACK:
				{

				}
					break;
				case SYN:
				{
					sendPacket.flags=SYN_ACK;
					sendPacket.windowsize=16;
					sendPacket.id=++uniqueIdentifier;
					sendto(s, (void *) &sendPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, slen);
				}
					break;
				default:
					break;
			}
		}
	}

	close(s);
	return 0;
}