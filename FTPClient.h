/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FTPClient.h
 * Author: rezan
 *
 * Created on June 7, 2016, 11:08 PM
 */

#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "Socket.h"
#include <string.h>
#include <iostream>
#include <sys/poll.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <stdio.h>

#define BUFSIZE 8192

class FTPClient {
public:
    FTPClient();                    //default constructor
    FTPClient(char*, char*, char*); //constructor with username and pass
    FTPClient(char* );              //constructor that is used
    ~FTPClient();                   //destructor
    int open_connection(char* hostName, int port);  //open the connection
    void close_connection();                        //close the connection
    void quit();                                    //quit the program
    int login(char *username, char* password);      //login/not used currently
    int sendUserName(char* nameToSend);             //send USER
    int sendMessage(const char *buffer);                  //send mesasge
    char* recvMessage();                            //receive message
    int sendPassword(char* passToSend);             //send PASS
    int sendPASV();         // Send PASV to server and obtain a new Server Socket
    int sendSYST();         // send client file system type
    int getPortFromPASV(char* );                    //get port from PASV
    bool changeDir(char* dirName);                  //change working dir
    char* getCurrentDirContents(); //returns buffer with directory contents
    int downloadFile(char *filename);               //get command
    bool putFile(char* fileName);                   //put command
    bool listDir(char* pathname);                   //list command
    int getReturnCode(char *message);               //get the return code
    double time_diff(struct timeval x, struct timeval y);   //get time
    int getMessageSize(char *msg);                  //get message size
    bool renameFile(char* oldFilename, char* newFilename);  //rename file
    bool makeDir(char* dirName);                    //make directory
    bool removeDir(char* dirName);                  //remove directory
    bool printWorkingDirectory();                   //PWD
    bool deleteFile(char* fileName);                //delete file


private:
    int clientSD;               //stores cliends SD
    int dataSD;                 //stores passive connection SD
    int recvBytes;              //
    Socket* sock;               //client socket
    Socket* serverSock;         //server socket
    char ctrlBuf[BUFSIZE + 1];  //
    char dataBuf[BUFSIZE + 1];  //
};



#endif /* FTPCLIENT_H */

