/**
 * @file
 * brief description, full stop.
 *
 * long description, many sentences.
 * 
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "udpserver.h"

UDPServer::UDPServer(){
    fd = -1;
    listener = NULL;
}

int UDPServer::start(int port){
    sockaddr_in addr;
    fd = socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0)
        return errno;
    
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    if(bind(fd,(sockaddr *)&addr,sizeof(addr))){
        close(fd);
        fd=-1;
        return errno;
    }
    return 0;
}

void UDPServer::stop(){
    if(fd>=0)
        close(fd);
}

void UDPServer::parseMessage(const char *s){
    char key[64],val[64],*p;
    while(*s){
        // skip spaces
        while(*s && isspace(*s))
            s++;
        // read the key name
        p=key;
        while(*s && *s!='=' && (p-key)<64)
            *p++=*s++;
        *p=0;
        if(!*s)
            break;// invalid, key at end of string
        
        s++; // skip the '='
        // read the value
        p=val;
        while(*s && !isspace(*s) && (p-val)<64)
            *p++=*s++;
        *p=0;
        
        listener->onKeyValuePair(key,atof(val));
    }
}

void UDPServer::poll(){
    char buf[1024];
    sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    int n = recvfrom(fd,buf,1024,MSG_DONTWAIT,(struct sockaddr *)&cliaddr,&len); 
    if(n>0 && listener){
        parseMessage(buf);
    }
}
