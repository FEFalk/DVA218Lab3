
#include "shared.h"

void sendData(int windowsize, int packetSize, )
{
    int totalActivePackets=0;
    int currentSequenceNum=0;
    while(1)
    {
        for(;totalActivePackets<windowsize && currentSequenceNum<packetSize; totalActivePackets++)
        {
            nbytes = sendto(...);
            if(nbytes < packetSize)
            //error
        }

        select(...);
        if(//select timed out)
            //resend correct packet/frame
                else
        recvfrom(...);

        switch(receivedPacket.flags)
        {
            case ACK:
            {
                //check sequence number etc.
                currentSequenceNum=highestSequenceNumberReceived;
                totalActivePackets--;
            }
                .
                .
                .
            default:
            {

            }
        }
    }
}
