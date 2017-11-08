/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Socket.h
 * Author: rezan
 *
 * Created on June 7, 2016, 11:09 PM
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
class Socket {

    public:
        Socket(int);
        int getServerSocket( );
        int getClientSocket(char*);

    private:
        int portNumber;
        int SD;
};


#endif /* SOCKET_H */

