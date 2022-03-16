//
// Created by apiers on 3/16/20.
//
#include <iostream>

/*For Serial IO*/

#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "udpclient.h"

#include "LN2Controller.h"
#include "SerialDeviceT.hpp"
#include "MysqlCredentials.hpp"
#include <mysqlx/xdevapi.h>

#define PORT "8888" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at onc
#define std::string hostname{"192.168.0.4"}


LN2Controller::LN2Controller(){


    this->WatchdogFuse = 1;
    this->CurrentLN2Valve = 6;
    this->CurrentLN2ValveState = 0;
    this->RTDVoltage = 0;
    this->LN2Interlock = 0;	// double check with Alex

    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];
    int client_struct_length = sizeof(client_addr);

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(socket_desc < 0){
        printf("Error while creating socket\n");
//        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
//        return -1;
    }
    printf("Done with binding\n");

    printf("Listening for incoming messages...\n\n");

    // Receive client's message:
    if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
         (struct sockaddr*)&client_addr, &client_struct_length) < 0){
        printf("Couldn't receive\n");
//        return -1;
    }
    printf("Received message from IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    printf("Msg from client: %s\n", client_message);

    // Respond to client:
    strcpy(server_message, client_message);

    if (sendto(socket_desc, server_message, strlen(server_message), 0,
         (struct sockaddr*)&client_addr, client_struct_length) < 0){
        printf("Can't send\n");
//        return -1;
    }

    // Close the socket:
    close(socket_desc);

//    return 0;
}


//LN2Controller::LN2Controller(std::string){
//
//
//    this->WatchdogFuse = 1;
//    this->CurrentLN2Valve = 6;
//    this->CurrentLN2ValveState = 0;
//    this->RTDVoltage = 0;
//    this->LN2Interlock = 0;	// double check with Alex
//}

LN2Controller::~LN2Controller(){
    close(USB);
};

// Read functions
void LN2Controller::ReadRTDVolatge(){
	char sendingBuffer[2] = {'r', '0'};
}

// Write Functions
void LN2Controller::WriteValve(){
    char sendingBuffer[2] = {'v', (char) this->CurrentLN2Valve};


    return;
}

void LN2Controller::WriteValveState(){
	char sendingBuffer[2] = {'s', (char) this->CurrentLN2ValveState};

    return;
}

void LN2Controller::SendHeartbeat(){
	char sendingBuffer[2] = {'h', '0'};
}

void LN2Controller::UpdateMysql(void ){
    // Connect to  Database
    mysqlx::Session DDroneSession("localhost", 33060, DMysqlUser, DMysqlPass);
    mysqlx::Schema DDatabase = DDroneSession.getSchema("DAMICDrone");

    // Get watchdog fuse from control parameters
    mysqlx::Table CtrlParameterTable = DDatabase.getTable("ControlParameters");
    mysqlx::RowResult CtrlRowResult = CtrlParameterTable.select("WatchDogFuse", "LN2ValveInterLock", "LN2ValveState", "CurrentLN2Valve")
            .bind("IDX", 1).execute();
    mysqlx::Row CtrlRow = CtrlRowResult.fetchOne();
    this->WatchdogFuse = (bool) CtrlRow[0];
    this->LN2Interlock = (bool) CtrlRow[1];
    this->ValveState = (int) CtrlRow[2];
    this->CurrentValve = (int) CtrlRow[3];

    // Get the LN2 table to update and insert values
    mysqlx::Table LN2ControllerTable = DDatabase.getTable("LN2ControllerState");
    mysqlx::Result LN2ControllerResult = LN2ControllerTable.insert("CurrentValve", "CurrentValveState", "RTDVoltage")
            .values(this->CurrentValve, this->ValveState, this->RTDVoltage).execute();

    // Check to see if values were inserted properly
    unsigned int warnings = LN2ControllerResult.getWarningsCount();
    (warnings == 0) ? this->SQLStatusMsg = "OK" : this->SQLStatusMsg = "WARN!\n";

    // Close the database connection
    DDroneSession.close();

    return;
}

void LN2Controller::PrintStatus(){
    printf("\rLiquid Nitrogen Control | Valve State: %i, Overflow Pin (V): %0.2f, Valve Interlock: %i, Valve Open: %i",
        CurrentLN2ValveState, RTDVoltage, LN2Interlock, CurrentLN2ValveState && !LN2Interlock);
    advance_cursor();
}
