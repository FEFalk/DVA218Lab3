//
// Created by fille on 2016-02-26.
//

#ifndef TESTPROJECT_SHARED_H
#define TESTPROJECT_SHARED_H

#define BUFLEN 512
#define NPACK 10
#define PORT 9932

#define DATA 0
#define ACK 1
#define SYN 2
#define SYN_ACK 3
#define FIN 4
#define FIN_ACK 5

typedef struct rtp_struct {
    int flags;
    int id;
    int seq;
    int windowsize;
    int crc;
    char *data;
} rtp ;


void diep(char *s);



#endif //TESTPROJECT_SHARED_H
