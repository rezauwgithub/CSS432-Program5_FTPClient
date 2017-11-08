/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FTPClient.cpp
 * Author: rezan
 * 
 * Created on June 7, 2016, 11:08 PM
 */

#include "FTPClient.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <sys/poll.h>
#include <sys/time.h>

#define BUFSIZE 8192

using namespace std;

//-----------------------------------------------------------------------------
FTPClient::FTPClient() {
    clientSD = 0;
    dataSD = 0;
    recvBytes = 0;
}

//-----------------------------------------------------------------------------
FTPClient::FTPClient(char* url, char* user, char* pass) {
    clientSD = 0;
    dataSD = 0;
    recvBytes = 0;
    while(open_connection(url, 21) <= 0){
       std::cout << "Cant connect. Reenter url";
       cin >> url;
    }
    login(user, pass);
}

//-----------------------------------------------------------------------------
FTPClient::FTPClient(char* url ) {
    clientSD = 0;
    dataSD = 0;
    recvBytes = 0;
    while(open_connection(url, 21) <= 0){
       std::cout << "Cant connect. Reenter url";
       cin >> url;
    }
}

//-----------------------------------------------------------------------------
FTPClient::~FTPClient() {
}

int FTPClient::open_connection(char* hostName, int port) {

    // Setup
    char buffer_in[1450];
    bzero(buffer_in,1450);

     // Attempt to connect to server
    sock = new Socket(port);
    // cout << "opened new socket, hostBuf: " << endl;
    clientSD = sock->getClientSocket(hostName);
    // cout << "about to recvMessage" << endl;
    strcpy( buffer_in, recvMessage() );
    // cout << "done recvMessage" << endl;
    std::cout << buffer_in << std::endl;
    while(getReturnCode(buffer_in) != 220) {
       std::cout << "Incorrect hostname and port: Enter new one" <<  std::endl;
       strcpy( buffer_in, recvMessage() );
    }
 
    // cout << "done in open connection" << endl;
    return clientSD; //true if > 0
}

//-----------------------------------------------------------------------------
//used to close the connection with server
void FTPClient::close_connection() {
    int code;
    char* msgptr; 
    char buffer[BUFSIZE];

    //add quitto buffer to be sent
    strcpy(buffer, "QUIT");  

    //send MKD, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
       exit(1); //might as well leave
    }  

    //Get message from server
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;

    close(clientSD);
}

//-----------------------------------------------------------------------------
//used to close and quit the program
void FTPClient::quit() {
    close(clientSD);
    exit(1);
}

//-----------------------------------------------------------------------------
// This funncction will take in a username and password, and will send them
// to the ftp server, checking to make sure the return code works. If it 
// doesn't, it asks the user to reenter
//----------------------------------------------------------------------------
int FTPClient::login(char *username, char *password) { 
    char buffer[BUFSIZE];
    bzero(buffer, BUFSIZE);
    int code;
    //If anything is wrong, have the user reenter their information
    do {
        code = sendUserName(username);
        if(code != 331) {
            cout << "Incorrect username. Reenter:";
            cin >> username;
        }

        if((code = sendPassword(password)) == 230)
        break;
    
        cout << "Incorrect password. Reenter:";
        cin >> password;
    } while (true);

    strcpy(buffer, recvMessage());
    cout << buffer << endl;
    return code;
}

//-----------------------------------------------------------------------------
//Returns 331 if successful
//Sends the mmessage "USER " and the username
//-----------------------------------------------------------------------------
int FTPClient::sendUserName(char* username) {
    // cout << "in send user" << endl;
    int code;
    memset( ctrlBuf, '\0', sizeof( ctrlBuf ) );
    char buffer[100];
    strcpy(buffer, "USER ");
    strcat(buffer, username);

    if(sendMessage(buffer) < 0) {
        perror("Can't send message\n");
        return 1;
    }
    
    strcpy(buffer, recvMessage());
    cout << buffer << endl; 

    return getReturnCode(buffer);
}

//-----------------------------------------------------------------------------
//get the first 3 chars from the received message and conver to an int to 
//get a returnable error code to check
int FTPClient::getReturnCode( char *message) {
    char temp[4];
    if(message == NULL || strlen(message) <= 3)
       return 0;
    for(int i = 0; i < 3; i++)
        temp[i] = message[i];
    temp[3] = '\0';
    return atoi(temp);
}

//-----------------------------------------------------------------------------
//Returns 230 if successful
//Sends a password to the server, and then system information 
//----------------------------------------------------------------------------
int FTPClient::sendPassword(char* password) {

    // cout << "in send pass" << endl;
    int code;
    char buffer[2048];
    strcpy(buffer, "PASS ");
    strcat(buffer, password);
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return 1;
    }    
    strcpy(buffer, recvMessage());
    std::cout << buffer << std::endl;

    if(getReturnCode(buffer) == 501)
        return -1;

    strcpy(buffer, recvMessage());
    cout << buffer << endl; 
    
    sendSYST();
    return getReturnCode(buffer);
}

//-----------------------------------------------------------------------------
//setup passive connection
int FTPClient::sendPASV(){
    char buffer[70];                            //allocate buffer
    bzero(buffer, 70 );                         //zero out buffer

    sendMessage("PASV ");                       //send the message PASV
    strcpy( buffer, recvMessage() );            //receive reply

    int port = getPortFromPASV( buffer );           //get port # from reply

    char url[] = "ftp.tripod.com";              //set the url to connect to

    serverSock = new Socket(port);              //new socket with new port #
    dataSD = serverSock->getClientSocket(url);  //new data socket

    return 0;
}

//-----------------------------------------------------------------------------
//send SYST command
int FTPClient::sendSYST(){

    int code;
    char buffer[2048];
    strcpy(buffer, "SYST ");
    // strcat(buffer, password);
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return 1;
    }    
    strcpy(buffer, recvMessage());
    std::cout << buffer << std::endl;
    return getReturnCode(buffer);
}

//-----------------------------------------------------------------------------
//Sends a message to the server with \r\n at the end to dictate the end of
//the message
//----------------------------------------------------------------------------
int FTPClient::sendMessage(const char* buffer) {
    int message_length = strlen(buffer) + 2;
    char message[message_length];
    bzero(message, sizeof(message));
    strcpy(message, buffer);
    strcat(message,  "\r\n");
    return write(clientSD, message, message_length);
}

//-----------------------------------------------------------------------------
//used to receive message
char* FTPClient::recvMessage() {

    //Configure polling
    struct pollfd ufds;
    ufds.fd = clientSD;
    ufds.events = POLLIN;
    ufds.revents = 0;

    char *buffer;
    buffer = new char[BUFSIZE];
    std::string message;
    bzero(buffer, BUFSIZE);
    char *retMsg;
    int msg_size = 0, val;
    bool recieve = false; 
    char error[] = "Error receiving message";
    char buffer_in[BUFSIZE];
    for(int i = 0; i < BUFSIZE; i++) {
        buffer_in[i] = '\0';
    }

    val = poll(&ufds, 1, 1000);
    int counter = 0;

    while(val > 0){
        memset(buffer, '\0', BUFSIZE); 
        msg_size = read(clientSD, buffer, BUFSIZE);

        //got a message? save it
        if(msg_size > 0) {
            message.append(buffer);
        //is this the end of your message?
            if(buffer[msg_size-1] == '\n' && buffer[msg_size-2] == '\r')
                break; 
        //Is your message empty?
            if(buffer[0] == '\0')
                break;
        }
        if(counter > 100 )
            break;

        val = poll(&ufds, 1, 1000);
        counter++;
    }
  
    if( msg_size > 0) {
        retMsg = new char[message.length() + 1];
        strcpy(retMsg, message.c_str());
        int msg_size = strlen(retMsg);
        for(int i = msg_size - 1; i > msg_size - 3; i--)
        if(retMsg[i] == '\n' || retMsg[i] == '\r')
        retMsg[i] = '\0';
    return retMsg;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
//use to get the port from passive connection
int FTPClient::getPortFromPASV( char* buffer ) {

    std::cout << buffer << std::endl;       //display output
    int address[6];                         //hold address and port

    //clear data sturcture
    for(int i=0; i<6; i++)
        address[i] = 0;

    char tempBuf[4];        //temp buffer
    bzero(tempBuf, 6);      //zero out buffer

    char tempChar;          //temp character
    int j = 0,k = 0;        //set to zero

    //find numbers, and store
    for(int i=20; i < strlen(buffer) ;i++ ) {
        if(buffer[i] == ',' || buffer[i] == ')') {
            address[j] = atoi(tempBuf);
            j++;
            k = 0;
            bzero(tempBuf, 6);
        }
        else {
            if( isdigit( buffer[i] )) {
                tempBuf[k] = buffer[i];
                k++;
            }

        }
    }

    //calculate port number
    int port = address[4] * 256 + address[5];

    return port;    //return port number
}

//-----------------------------------------------------------------------------
//use to change the working directory
bool FTPClient::changeDir(char* dirName) {

    int code;
    char* msgptr; 
    char buffer[BUFSIZE];

    //add CWD to buffer to be sent
    strcpy(buffer, "CWD ");  
    if(dirName != NULL) 
        strcat(buffer, dirName);

    //send CWD, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return false;
    }  

    //Get message from server
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;

    return true;
}

//-----------------------------------------------------------------------------
//List command
char* FTPClient::getCurrentDirContents() {

    int code;
    char* dataptr;
    char* msgptr; 
    char buffer[BUFSIZE];
    bzero(buffer, BUFSIZE);

    //get PASV connection setup with dataSD
    sendPASV(); 

    //save clientSD temporarily, we will need it                
    int tempSD = clientSD;      

    //add LIST to buffer to be sent
    strcpy(buffer, "LIST");                 

    //send LIST, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return NULL;
    }  

    //Get message from server
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;


    //set clientSD to dataSD
    clientSD = dataSD; 

    //recieve data buffer from server
    dataptr = recvMessage();
    if(dataptr == NULL) {
        // cout << "is it NULL???" << endl;
        clientSD = tempSD; 
        cout << recvMessage() << endl;
        close(dataSD);
        return dataptr;
    }
    std::cout << dataptr << std::endl;
    
    //set clientSD to itself again
    clientSD = tempSD;   

    //Recieve end of stream message
    msgptr = recvMessage();  
    std::cout << msgptr << std::endl;

    //close PASV connection
    close( dataSD );                        

    //return buffer with directory contents
    return dataptr;

}

//-----------------------------------------------------------------------------
//Get command
int FTPClient::downloadFile(char *filepath) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int code;
    char* dataptr;
    char* msgptr;
    char buffer[BUFSIZE];
    char typeBuffer[BUFSIZE];
    bzero(buffer, BUFSIZE);
    //XXX: This should read from the filepath itself!!!!
    string filename = string(filepath);
    //save clientSD temporarily, we will need it
    int tempSD = clientSD;
    cout << "local: " << filename << " remote: " << filename << endl;
    sendMessage("Type I");
    strcpy(typeBuffer, recvMessage());
    cout << typeBuffer << endl;

    // check if we set the mode to Binary successfully(code 200)
    code = getReturnCode(typeBuffer);
    if(code != 200) {
        cout << "downloadFile() error" << endl;
        return code;
    }   
    sendPASV();                     //set up passive connection
    char msg[2048];
    strcpy(msg, "RETR ");
    strcat(msg, filename.c_str());
    sendMessage(msg);               //send RETR with fileName

    strcpy(buffer, recvMessage());
    cout << buffer << endl;  //get 150 message or 550(no such file)
    code = getReturnCode(buffer);
    if(code == 550) {
        close(dataSD);
        clientSD = tempSD;          //set clientSD back to itself
        return code;
    }
    int size = 0, filesize = 0;
    //open file
    std::ofstream file(filename.c_str(), ios_base::out | ios::binary);

    //set up time structure, and get start time
    struct timeval start;
    gettimeofday(&start, NULL);

    //set up polling stuff
    struct pollfd ufds;
    ufds.fd = dataSD;
    ufds.events = POLLIN;
    ufds.revents = 0;
    
    int val = poll(&ufds, 1, 1000);
    //while something to read
    while(val > 0) {
        memset(buffer, '\0', BUFSIZE);
        size = read(dataSD, buffer, BUFSIZE);
    //something to read
        if(size > 0) {
            filesize += size;
            file.write(buffer, size);
        }
        else
            break;
    //end of read?
        if(buffer[size-1] == '\n' && buffer[size-2]=='\r')
            break;
        if(buffer[0] == '\0')
            break;
        val = poll(&ufds, 1, 1000);
    }
    //error
    if(filesize == 0) {
        cout << "get file size was 0!" << endl;
        file.close();
        close(dataSD);
        recvMessage();
        return 0;
    }
    //start timer
    struct timeval end, diff;
    gettimeofday(&end, NULL); 
    //restore connnection
    clientSD = tempSD;          //set clienSD back to itself
    file.close();               //close file stream
    close(dataSD);              //close data connection
    char *mess = recvMessage();         //get 226 message
    int total_bytes = getMessageSize(mess);     //get 226 message
    cout << mess << endl;               

    cout << total_bytes << " bytes received in " << time_diff(start, end);
    cout << " milliseconds (" << (total_bytes * 1000000) / time_diff(start, end);
    cout << " Kbytes/s)" << endl;   

    return filesize;
}

//-----------------------------------------------------------------------------
//pull the message size in bytes from return message from server
int FTPClient::getMessageSize(char *msg) {
    int start = 0, end = 0;
    char array[1000];
    //Get the index of (
    for(int i = 0; i < 5000; i++) {
        if(msg[i] == '(' ) {
            start = i;
            start++;
            end = start;
            while(msg[end] != ' ') {
                end++;
            }
        }
    }

    memcpy( array, msg+start, (end)-start );
    return atoi(array);
}

//-----------------------------------------------------------------------------
//get total time taken 
double FTPClient::time_diff(struct timeval x, struct timeval y) {
    double x_ms, y_ms, diff;
    x_ms = (double)x.tv_sec*1000000 + (double)x.tv_usec;
    y_ms = (double)y.tv_sec*1000000 + (double)y.tv_usec;
    diff = (double)y_ms - (double)x_ms;
    return diff;
}

//-----------------------------------------------------------------------------
// put command
bool FTPClient::putFile(char* fileName) {

    struct pollfd ufds;
    ufds.fd = clientSD;
    ufds.events = POLLIN;
    ufds.revents = 0;

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int code;
    char* dataptr;
    char* msgptr;
    char buffer[BUFSIZE];
    char fileBuffer[BUFSIZE];
    bzero(buffer, BUFSIZE);
    string filename = string(fileName);

    //save clientSD temporarily, we will need it
    int tempSD = clientSD;

    cout << "local: " << filename << " remote: " << filename << endl;

    FILE *file;
    int file_size;
    file = fopen(filename.c_str(), "rb");
    if(file) {
        file_size = fread(fileBuffer, BUFSIZE, 1, file);
    }
    else {
        cout << "local: " << filename.c_str() << ": No such file or directory\n";
        clientSD = tempSD;
        return 0;
    }
    //set up binary
    sendMessage("Type I");
    strcpy(buffer, recvMessage());
    cout << buffer << endl;
    code = getReturnCode(buffer);
    if(code != 200) {
        cout << "put() error" << endl;
        return code;
    }   
    //set up data connection
    sendPASV();                     //setup passive connection
    char msg[2048];                 //
    strcpy(msg, "STOR ");           //
    strcat(msg, filename.c_str());  //
    sendMessage(msg);               //send message STOR and fileName
    //retrive response confirmation 
    cout << recvMessage() << endl;  //get 150 message or 550(no such file)
    clientSD = dataSD;
    struct timeval start, end, diff;
    gettimeofday(&start, NULL);
    sendMessage(fileBuffer);
    gettimeofday(&end, NULL); 

    clientSD = tempSD;
    fclose(file);
    close(dataSD);
    //download
    char *mess = recvMessage();             //get 226 message with (xx bytes sent)
    int total_bytes = getMessageSize(mess);
    cout << mess << endl;
    cout << total_bytes << " bytes received in " << time_diff(start, end);
    cout << " ms (" << (total_bytes * 1000000) / time_diff(start, end) << "Kbytes/s)" << endl;   

    return file_size;
}

//-----------------------------------------------------------------------------
//Rename command
bool FTPClient::renameFile(char* oldFilename, char* newFilename){

    int code;
    char* msgptr; 
    char buffer[BUFSIZE];

    //add file name to be changed to send buffer
    strcpy(buffer, "RNFR ");  
    if(oldFilename != NULL) 
        strcat(buffer, oldFilename);

    //send RNFR, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return false;
    }  
    
    msgptr = recvMessage();
    //std::cout << msgptr << std::endl;

    //fill buffer with RNTO and filename to change to
    strcpy(buffer, "RNTO ");  
    if(newFilename != NULL) 
        strcat(buffer, newFilename);

    //send RNTO, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return false;
    }

    //Get message from server
    delete [] msgptr; 
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;
    delete [] msgptr; 
    return true;
}

//make directory command
bool FTPClient::makeDir(char* dirName){
    
    int code;
    char* msgptr; 
    char buffer[BUFSIZE];

    //add MKD to buffer to be sent
    strcpy(buffer, "MKD ");  
    if(dirName != NULL) 
        strcat(buffer, dirName);

    //send MKD, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return false;
    }  

    //Get message from server
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;

    return true;
}

//remove directory command
bool FTPClient::removeDir(char* dirName){
    
    int code;
    char* msgptr; 
    char buffer[BUFSIZE];

    //add RMD to buffer to be sent
    strcpy(buffer, "RMD ");  
    if(dirName != NULL) 
        strcat(buffer, dirName);

    //send RMD, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return false;
    }  

    //Get message from server
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;

    return true;
}

//print workind directory
bool FTPClient::printWorkingDirectory(){
    int code;
    char* msgptr; 
    char buffer[BUFSIZE];

    //add PWD to buffer to be sent
    strcpy(buffer, "PWD");  

    //send PWD, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
       exit(1); //might as well leave
    }  

    //Get message from server
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;
}

//delete file command
bool FTPClient::deleteFile(char* fileName){
    int code;
    char* msgptr; 
    char buffer[BUFSIZE];

    //add DELE to buffer to be sent
    strcpy(buffer, "DELE ");  
    if(fileName != NULL) 
        strcat(buffer, fileName);

    //send DELE, output error if there was one, message sent on clientSD
    if(sendMessage(buffer) < 0) {
       perror("Can't send message\n");
        return false;
    }  

    //Get message from server
    msgptr = recvMessage();
    std::cout << msgptr << std::endl;

    return true;
}

