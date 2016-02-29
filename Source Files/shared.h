
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
using namespace std;

#define BUFLEN 512
#define PORT 9932

#define DATA 0
#define LAST_DATA 1
#define ACK 2
#define SYN 3
#define SYN_ACK 4
#define FIN 5
#define FIN_ACK 6


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
