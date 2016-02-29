
#include "wrapper.h"
#include "shared.h"
using namespace std;

int main(void)
{
    setbuf(stdout, NULL);
	struct sockaddr_in si_me, si_other;
	int s, i;
	bool isConnected=false;
	socklen_t slen = sizeof si_other;
	rtp sendPacket;
	rtp *recvPacket = (rtp *)calloc(sizeof(rtp), 1);
	recvPacket->data = (char *)calloc(sizeof(char), 256);
	char errorMessage[512];
	int uniqueIdentifier=0;
	int nbytes;
	int packetSize;
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

	struct timeval timeout={2, 0};

	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

	if(!isConnected)
	{
		connectTo(s, si_other, &uniqueIdentifier);
		isConnected=true;
	}

	//Disable timeout to wait for initial DATA/FIN-packets infinitely
	timeout={0, 0};
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
	recvfrom(s, recvPacket, BUFLEN, 0, (struct sockaddr*) &si_other, &slen);

	//Enable timeout
	timeout={2, 0};
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
	while(isConnected)
	{
		switch(recvPacket->flags)
		{
			case DATA:
			{
				if(recvDataFrom(s, (struct rtp *)recvPacket, si_other)==-1)
				{
					terminateProgram(s);
					return -1;
				}

			}
				break;
			case FIN:
			{
				sendPacket.flags=FIN_ACK;
				sendPacket.windowsize=16;
				sendPacket.id=++uniqueIdentifier;
				sendto(s, (void *) &sendPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, slen);
			}
				break;
			default:
				break;
		}
	}

	close(s);
	return 0;
}