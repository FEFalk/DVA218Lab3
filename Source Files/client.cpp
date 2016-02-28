
#include "shared.h"
#include "wrapper.h"


#define SERVER_IP "127.0.0.1"

int main(void)
{
    struct sockaddr_in si_server, si_other;
    int s, i;
    socklen_t slen=sizeof si_server;
    int uniqueIdentifier, windowSize;
    char *errorMessage;
    int nbytes;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    sendPacket->data = (char *)calloc(sizeof(char), 256);
    rtp recvPacket;


    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
        strcpy(errorMessage, "Failed to create socket");
        diep(errorMessage);
    }


    memset((char *) &si_server, 0, sizeof(si_server));

    si_server.sin_family = AF_INET;
    si_server.sin_addr.s_addr = inet_addr(SERVER_IP);
    si_server.sin_port = htons(PORT);

    if (inet_aton(SERVER_IP, &si_server.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


    sendPacket->flags=SYN;
    sendPacket->id=0;
    nbytes = sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);

    printf("%d\n", nbytes);

    recvfrom(s, &recvPacket, 256, 0, (struct sockaddr*) &si_other, &slen);

    if(recvPacket.flags==SYN_ACK)
    {
        uniqueIdentifier=recvPacket.id;
        windowSize=recvPacket.windowsize;

        sendPacket->flags=ACK;
        nbytes = sendto(s, (void *) sendPacket, sizeof(*sendPacket), 0, (struct sockaddr*) &si_server, slen);
    }

    printf("%d\n", nbytes);
    //TODO
    /*If timed out, proceed*/
    //recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, (socklen_t *)&slen);
    //if(recvPacket.flags==ACK)
    //{
    //    printf("Connected to the server!");
    //}

    sendPacket->flags=DATA;
    printf("Sending packet 1\n");
    strcpy(sendPacket->data, "Hello 1 :^)");
    //Change size to something more logical
    nbytes = sendto(s, (void *)sendPacket, 256, 0, (struct sockaddr*) &si_server, slen);

    string msg;
    int packetSize=0;
    cout << "You are now connected to the server! \n"
                   "Enter a message to send to the server, type END to quit.\n";
    while(1)
    {
        cout << " >";
        getline(cin, msg);
        //Some error-handling
        if(msg.length() > 255)
            msg[255]=0;
        else if(msg.length() <= 0)
        {
            cout << "Please enter a message to send to the server.\n"
            continue;
        }
        //Some error-handling
        if(msg.length() > 255)
            msg[255]=0;

        //If user wants to terminate connection
        if(msg.compare("END")==0)
        {
            //Send FIN-packet to server to close connection
            break;
        }

        cout << "Enter how many packets to be sent to the server\n";
        while(1)
        {
            cout << " >";
            cin >> packetSize;
            if(packetSize > 0)
                break;
            else
                cout << "Wrong input! Please enter how many packets to be sent to the server\n";
        }

        cout << "\nStarting transfer of " << packetSize << " packets to server...\n";
        //Send DATA-packet to server
        sendData()

    }



    printf("%d\n", nbytes);
    for (i=2; i<NPACK; i++) {
        recvfrom(s, &recvPacket, sizeof(rtp), 0, (struct sockaddr*) &si_other, (socklen_t *)&slen);
        if(recvPacket.flags==ACK)
        {
            printf("Sending packet %d\n", i);
            fflush(stdout);
            sprintf(sendPacket->data, "Hello %d :^)", i);
            if((sendto(s, (void*) sendPacket, 256, 0, (struct sockaddr*) &si_server, slen)) == -1)
            {
                strcpy(errorMessage, "Failed the 'sendto()'");
                diep(errorMessage);
            }
            printf("%s", errorMessage);
        }
    }

    close(s);
    return 0;
}