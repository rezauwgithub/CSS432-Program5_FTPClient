/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ftp.cpp
 * Author: rezan
 *
 * Created on June 7, 2016, 11:07 PM
 */

#include "FTPClient.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <string>
#define CHAR_SIZE 1000
#define MAX_FTP_ARGS 10

char** userInput;
int inputSize = 0; 
char* serverIP;
FTPClient* client;
std::stringstream prompt;
//-----------------------------------------------------------------------------
///This function presents the user with a command line then stores the
// results in char** userInput as indiviual tokens. the number of tokens is
// given by the global variable inputSize.
char** getUserInput() {
    if(userInput){
        delete userInput; 
    }
    userInput = new char*;
    char* temp;
    char input[CHAR_SIZE];

    inputSize = 0;
    memset(input, '\0', sizeof(input));
    std::cout << prompt.str(); 
    std::cin.clear(); 
    std::cin.getline(input, CHAR_SIZE, '\n');

    userInput[inputSize] = strtok(input, " ");

    while(userInput[inputSize] != NULL) {
        inputSize++;
        userInput[inputSize] = strtok(NULL, " \n");
    }
    return userInput;
}


//-----------------------------------------------------------------------------
void outputHelp() {
    std::cout << "./ftp hostName" << std::endl;
}

//-----------------------------------------------------------------------------
void outputHelp2() {
    std::cout << "Commands may be abbreviated. Commands are: \n" << std::endl;
    std::cout << "cd        ls       quit    " << std::endl;
    std::cout << "close     mkdir    rename  " << std::endl;
    std::cout << "delete    put      rmdir   " << std::endl;
    std::cout << "get       pwd              " << std::endl;
    // std::cout << "get       put         close" << std::endl;
}

//checks for what command the client wants to execute according to input
bool execCommand(bool& connected){

    if(userInput[0] == NULL)
        return true;

    if(connected){
    //--------- CLIENT CONNECTED

        if(!strcmp(userInput[0], "cd"))
            client->changeDir(userInput[1]);
        else if(!strcmp(userInput[0], "open")) {
            //XXX 21 should be taken from command line too
            int port = 21; 
            if(userInput[2] != NULL){
                port = atoi(userInput[2]);
            }

            std::cout << userInput[1] << std::endl; 
            while(client->open_connection(userInput[1], port) <= 0){
                std::cout << "Cant connect. Reenter url";
                //std::cin >> userInput[1];
            }
            prompt.str("");
            std::string userString( getlogin() );
            std::cout << "Name (" << userInput[1] << ":" << userString << "): ";
            getUserInput();

            client->sendUserName(userInput[0]);
            std::cout << "Password: ";
            getUserInput();
            client->sendPassword(userInput[0]);
            connected = true; 
            prompt.str("ftp> ");
        }
        else if(!strcmp(userInput[0], "ls"))
            client->getCurrentDirContents();
        else if(!strcmp(userInput[0], "get"))
            client->downloadFile(userInput[1]);
        else if(!strcmp(userInput[0], "put"))
            client->putFile(userInput[1]);
        else if(!strcmp(userInput[0], "close")){
            client->close_connection();
            std::cout << "Client disconnected\n";
            connected = false;
        }

        else if(!strcmp(userInput[0], "mkdir"))
            client->makeDir(userInput[1]);
        else if(!strcmp(userInput[0], "rmdir"))
            client->removeDir(userInput[1]);
        else if(!strcmp(userInput[0], "delete"))
            client->deleteFile(userInput[1]);
        else if(!strcmp(userInput[0], "rename"))
            client->renameFile(userInput[1], userInput[2]);
        else if(!strcmp(userInput[0], "exit"))
            client->quit();
        else if(!strcmp(userInput[0], "quit"))
            client->quit();
        else if(!strcmp(userInput[0], "pwd"))
            client->printWorkingDirectory();
        else if(!strcmp(userInput[0], "name"))
            client->sendUserName(userInput[1]);
        else if(!strcmp(userInput[0], "password"))
            client->sendPassword(userInput[1]);
        else if(!strcmp(userInput[0], "help") || !strcmp(userInput[0], "?"))
            outputHelp2(); 
        else
            std::cout << "INVALID COMMAND - Please re-enter or type (?)" << std::endl;

    }else{
    //-------- CLIENT DISCONNECTED

        if(!strcmp(userInput[0], "open")) {
            //XXX 21 should be taken from command line too
            int port = 21; 
            if(userInput[2] != NULL){
                port = atoi(userInput[2]);
            }

            while(client->open_connection(userInput[1], port) <= 0){
                std::cout << "Cant connect. Reenter url";
                std::cin >> userInput[1];
            }
            prompt.str("");
            std::string userString( getlogin() );

            do {
                std::cout << "Name (" << serverIP << ":" << userString << "): ";
                getUserInput();
            }while( userInput[0] == NULL );

            client->sendUserName(userInput[0]);
            do {
                std::cout << "Password: ";
                getUserInput();
            }while( userInput[0] == NULL );
            client->sendPassword(userInput[0]);
            connected = true; 
            prompt.str("ftp> ");
        }
        else if(!strcmp(userInput[0], "help") || !strcmp(userInput[0], "?"))
            outputHelp();
        else if(!strcmp(userInput[0], "exit") || !strcmp(userInput[0], "quit") || !strcmp(userInput[0], "close")){
            client->quit();
            return false;
        }else{
            std::cout << "Client disconnected - type Open to connect or (?) for more options\n";
        }

    }
    return true; 
}

//-----------------------------------------------------------------------------
// run with ./ftp ftp.tripod.com
int main( int argc, char* argv[] ) {

    bool connected = false;             //set initially to false

    if(argc > 1){
        serverIP = argv[1];

        client = new FTPClient(argv[1]);
    
        std::string userString( getlogin() );

        do {
            std::cout << "Name (" << serverIP << ":" << userString << "): ";
            getUserInput();
        }while( userInput[0] == NULL );

        client->sendUserName(userInput[0]);

        do{
            do {
                std::cout << "Password: ";
                getUserInput();
            }while( userInput[0] == NULL );
        }while(client->sendPassword(userInput[0]) == -1);

        connected = true; 
    }
    else{
        client = new FTPClient();
    }

    

    prompt.str("ftp> ");

    do{
        getUserInput();
    }while(execCommand(connected));

    return 0;
}
