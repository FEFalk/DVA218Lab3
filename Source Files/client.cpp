
#include "shared.h"
#include "wrapper.h"


#define SERVER_IP "127.0.0.1"

int main(void)
{
    struct sockaddr_in si_server;
    int s;
    int uniqueIdentifier, windowSize;
    char *errorMessage;
    rtp *sendPacket = (rtp *)calloc(sizeof(rtp), 1);
    sendPacket->data = (char *)calloc(sizeof(char), 256);

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

    //TIMEOUT INITIALIZED
    struct timeval timeout={2, 0};
    //TIMEOUT INITIALIZED
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout, sizeof(timeout));

    if((uniqueIdentifier = connectTo(s, &windowSize, si_server)) == -1)
        return 0;


    cout << "You are now connected to the server! \n"
            "Enter a message to send to the server, type END to quit.\n";

    string msg;
    int packetSize=-1;

    while(1)
    {
        cout << " >";
        getline(cin, msg);
        //Some error-handling
        if(msg.length() > 255)
            msg[255]=0;
        else if(msg.length() <= 0)
        {
            cout << "Please enter a message to send to the server.\n";
            continue;
        }
        //Some error-handling
        if(msg.length() > 255)
            msg[255]=0;

        //If user wants to terminate connection
        if(msg.compare("END")==0)
        {
            terminateProgram(s, uniqueIdentifier, si_server);
            return 0;
        }
        //Copy the string to a char* - datatype
        char *cstr = (char *)calloc(sizeof(char), msg.length()+1);
        strcpy(cstr, msg.c_str());

        cout << "Enter how many packets to be sent to the server. Enter 0 to exit.\n";
        while(1)
        {
            cout << " >";
            cin >> packetSize;
            if(packetSize > 0)
                break;
            else if(packetSize == 0)
            {
                terminateProgram(s, uniqueIdentifier, si_server);
                return 0;
            }
            else
                cout << "Wrong input! Please enter how many packets to be sent to the server\n";
        }

        cout << "\nStarting transfer of " << packetSize << " packets to server...\n";

        //Send DATA-packet to server
        sendDataTo(s, uniqueIdentifier, cstr, windowSize, packetSize, si_server);
        free(cstr);
    }

}