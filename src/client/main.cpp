#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <algorithm>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <limits.h>
#include "../hw1_common.h"
using namespace std;

#define TCP "tcp"
#define UDP "udp"

bool validateIpAddress(const string ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

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
    uint number=0;
    string protocol;
    string hostName;
    uint optCount=0;
    uint optTotal=4;
    uint port;
    bool isHostIpAddress=false;
    int s;
    char outbuf[256];
    char inbuf[256];


    fprintf(stdout,"Welcome to hw1 client.\n");

     while ((c = getopt (argc, argv, "x:t:s:p:")) != -1)
     {
         switch (c)
         {
            case 'x':
                if(!isStringANumber(optarg)){
                    fprintf(stderr,"Option -x is invalid, please enter a valid number.\n");
                    return 0;
                }
                number = atoi(optarg);
                if(number == INT_MAX){
                    fprintf(stderr,"Option -x is invalid, please enter a valid number.\n");
                    return 0;
                }
                optCount++;

                break;
            case 't':
                protocol.assign(optarg);
                std::transform(protocol.begin(), protocol.end(),protocol.begin(), ::tolower);
                optCount++;
                if(protocol.compare(TCP) != 0 &&  protocol.compare(UDP) != 0){
                    fprintf(stderr,"Option -t is invalid, please enter either tcp or udp.\n");
                    return 0;
                }
                break;
            case 's':
                hostName.assign(optarg);
                optCount++;
                isHostIpAddress = validateIpAddress(hostName);
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


    //create the message...
    clientMessage message = {htons(1),htons(number)};
    replyMessage reply_message;

    //open a connection...
    struct sockaddr_in sin;
    struct hostent *hp;


    hp = gethostbyname(hostName.c_str());
    if(!hp){
        fprintf(stdout, "Unknown host: %s\n.", hostName.c_str());
        return 0;
    }


    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);

    sin.sin_port = htons(port);

    int sockType=0;
    if(protocol == TCP){
        sockType = SOCK_STREAM;
    }else{
        sockType = SOCK_DGRAM;
    }

    if((s =socket(PF_INET, sockType, 0)) < 0){
        perror("Error on socket.");
        return 0;
    }

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 3000000;

    if((connect(s, (struct sockaddr *)&sin, sizeof(sin))) < 0){
        perror("connect error.");
        close(s);
        return 0;
    }


    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(s,&read_fds);

    //send the message...
    bcopy((void*)&message, (void*)&outbuf, sizeof(struct clientMessage));
    outbuf[sizeof(struct clientMessage) + 1]='\n';
    if(send(s, (void*)&outbuf, sizeof(outbuf), 0) < 0){
        perror("send");
        return 0;
    }

    fprintf(stdout,"Sent number: %i to %s:%i via %s\n", number, hostName.c_str(), port, protocol.c_str());
    bzero((void *)outbuf, sizeof(outbuf));

    if (select(s+1, &read_fds, NULL, NULL, &tv) == -1) {
        perror("select");
        return 0;

    }

    if (FD_ISSET(s, &read_fds))
    {
        if(recv(s, (void *)&inbuf, sizeof(inbuf), 0) < 0){
            perror("receive");
            return 0;
        }

        bcopy(inbuf, (void *)&reply_message, sizeof(struct replyMessage));
        fprintf(stdout, "Success!\n");
    }
    else
    {
        fprintf(stderr,"Error! No reply.\n");
    }

    close(s);
    return 0;
}


