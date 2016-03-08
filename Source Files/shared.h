/**
    Datacommunication, DVA218 - Lab 3
    shared.h
    Purpose: Contains defines for the different flags used in packets.
    Also defines the used Window-size to use within the server.
    Includes the packet-struct skeleton.

    @author Filiph Eriksson-Falk - ffk13001, Fredrik Frenning - ffg12002
    @date 08/03/2016
*/
#ifndef TESTPROJECT_SHARED_H
#define TESTPROJECT_SHARED_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include "wrapper.h"


using namespace std;

#define BUFLEN 512
#define PORT 9932
#define WINDOWSIZE 5

#define DATA 1
#define LAST_DATA 2
#define ACK 3
#define SYN 4
#define SYN_ACK 5
#define FIN 6
#define FIN_ACK 7


typedef struct rtp_struct {
    int flags;
    int id;
    int seq;
    int windowsize;
    int crc;
    char *data;
} rtp;

void diep(char *s);


#endif //TESTPROJECT_SHARED_H
