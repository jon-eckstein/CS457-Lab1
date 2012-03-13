#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../hw1_common.h"
using namespace std;

#define TCP "tcp"
#define UDP "udp"
#define MAX_PENDING 256

bool isStringANumber(string val){

    for ( string::const_iterator i=val.begin(); i!=val.end(); ++i ){
        if (!isdigit(*i)){
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    char c;
    string protocol;
    uint optCount=0;
    uint optTotal=2;
    uint port;

    fprintf(stdout, "Welcome to hw1 server.\n");

    while ((c = getopt (argc, argv, "t:p:")) != -1){
        switch (c){

            case 't':
                protocol.assign(optarg);
                std::transform(protocol.begin(), protocol.end(),protocol.begin(), ::tolower);
                optCount++;
                if(protocol.compare(TCP) != 0 &&  protocol.compare(UDP) != 0){
                    fprintf(stderr,"Option -t is invalid, please enter either tcp or udp.\n");
                    return 0;
                }
                break;

            case 'p':
                if(!isStringANumber(optarg)){
                    fprintf(stderr,"Option -p is invalid, please enter a valid number.\n");
                    return 0;
                }
                port = atoi(optarg);
                optCount++;
                if(port < 1 || port > 65535){
                    fprintf(stderr,"Option -p is invalid, please enter a port number between 1 and 65535.\n");
                    return 0;
                }
                break;
        }
    }


    if(optCount != optTotal){
        fprintf(stderr, "One or more options are missing.  Please try again. \n");
        return 0;
    }

    struct sockaddr_in sin;
    int s;
    int new_s;
    socklen_t s_size;
    socklen_t c_size;
    char outbuf[256];
    char inbuf[256];

    bzero((char *)&sin,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    int sockType=0;
    if(protocol == TCP){
        sockType = SOCK_STREAM;
    }else{
        sockType = SOCK_DGRAM;
    }


    if((s = socket(PF_INET,sockType,0)) < 0){
        perror("socket error.");
        return 0;
    }

    int yes=1;
    if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        return 0;
    }


    if((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0 ){
         perror("Bind error");
         return 0;
    }

    if(sockType == SOCK_STREAM && listen(s, MAX_PENDING) < 0){
        perror("Listen error");
    }

    fprintf(stdout, "listening for %s messages on port %i...\n", protocol.c_str(), port);

    replyMessage reply_message = { htons(1) };
    clientMessage client_message;
    struct sockaddr_in *cin;
    cin = (sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    if(cin == NULL){
        fprintf(stderr,"no memory for allocation.");
        return 0;
    }

    /* wait for connection and send reply */
    s_size = sizeof sin;
    while(1){
        if(protocol == TCP){
            if((new_s = accept(s, (struct sockaddr *)&sin, &s_size)) < 0){
                perror("accept error");
                return 0;
            }
        }else{
            new_s=s;
        }

        int len;

        c_size = sizeof cin;
        while((len = recvfrom(new_s, (void *)&inbuf, sizeof(inbuf), 0, (struct sockaddr *)&cin, &c_size))){
            bcopy(inbuf, (void *)&client_message, sizeof(struct clientMessage));
            fprintf(stdout, "The number is: %i\n", ntohs(client_message.number));
            //send reply...
            bzero(outbuf, sizeof(outbuf));
            bcopy((void*)&reply_message, outbuf, sizeof(struct replyMessage));
            outbuf[sizeof(struct replyMessage) + 1] = '\n';
            if(sendto(new_s,outbuf, sizeof(outbuf), 0, (struct sockaddr *)&cin, sizeof(struct sockaddr)) < 0){
               perror("sendto");
               return 0;
            }
        }

        bzero((char *)&inbuf,sizeof(inbuf));

        if(protocol==TCP){
            close(new_s);
        }
    }


    return 0;
}
