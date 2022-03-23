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


#include "LN2Controller.h"
#include "SerialDeviceT.hpp"
#include "MysqlCredentials.hpp"
#include <mysqlx/xdevapi.h>


LN2Controller::LN2Controller(std::string server_ip) : UDPClient(server_ip){
    this->WatchdogFuse = 1;
    this->CurrentLN2Valve = 6;
    this->CurrentLN2ValveState = 0;
    this->RTDVoltage = 0;
    this->LN2Interlock = 0;
}


LN2Controller::~LN2Controller(){
    close(socket_fd);
};

// Read functions
void LN2Controller::ReadRTDVolatge(){
	char buffer[2] = {'r', '0'};
	char ReceivingBuffer = this->ExchangePackets(buffer);
	RTDVoltage = ReceivingBuffer[-1];

}

// Write Functions
void LN2Controller::WriteValve(){
    char buffer[2] = {'v', (char) this->CurrentLN2Valve};
    char ReceivingBuffer = this->ExchangePackets(buffer);

}

void LN2Controller::WriteValveState(){
	char buffer[2] = {'s', (char) this->CurrentLN2ValveState};
	char ReceivingBuffer = this->ExchangePackets(buffer);

}

void LN2Controller::SendHeartbeat(){
	char buffer[2] = {'h', '0'};
	char ReceivingBuffer = this->ExchangePackets(buffer);

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
    int IncomingValveState = (int) CtrlRow[2];
    int IncomingCurrentValve = (int) CtrlRow[3];


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
