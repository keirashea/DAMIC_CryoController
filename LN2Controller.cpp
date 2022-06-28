//
// Created by apiers on 3/16/20.
//
#include <iostream>

/*For Serial IO*/
#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <unistd.h>

#include "LN2Controller.h"
#include "SerialDeviceT.hpp"
#include "MysqlCredentials.hpp"
#include <mysqlx/xdevapi.h>


LN2Controller::LN2Controller(){
    this->WatchdogFuse = 1;
    this->CurrentLN2Valve = 6;
    this->CurrentLN2ValveState = 0;
    this->RTDVoltage = 0;
    this->LN2Interlock = 0;

    printf("Dummy instance of class. No serial communication available");
}

LN2Controller::LN2Controller(std::string SerialPort) : SerialDevice(SerialPort){

        /* Set Baud Rate */
        cfsetospeed (&this->tty, (speed_t)B9600);
        cfsetispeed (&this->tty, (speed_t)B9600);

        /* Setting other Port Stuff */

        // Found that the defaults in tty worked best for arduino communication

        /* Flush Port, then applies attributes */
        tcflush( USB, TCIFLUSH );
        if ( tcsetattr ( USB, TCSANOW, &tty ) != 0)
        {
            std::cout << "Error " << errno << " from tcsetattr" << std::endl;
        }


        this->WatchdogFuse = 1;
        this->CurrentLN2Valve = 6;
        this->CurrentLN2ValveState = 0;
        this->RTDVoltage = 0;
        this->LN2Interlock = 0;

        printf("Liquid Nitrogen Controller is now ready to accept instructions.\n");
}

LN2Controller::~LN2Controller(){
    close(USB);
};

// Read functions
void LN2Controller::ReadRTD(){
    std::string ArdLn2_String;
    std::string cmd = "r";

    this->WriteString(cmd);
    ArdLn2_String = this->ReadLine();

    try {
        this->RTDVoltage = std::stof(ArdLn2_String);
    } catch(...){
        printf("Error in Reading Overflow Voltage. Continuing...\n");
    }

}

// Write Functions
void LN2Controller::WriteValve(){

    // Sets the valve state on the arduino (1 = open, 0 = close)
    std::string cmd = "v" + std::to_string(CurrentLN2Valve && !LN2Interlock);

    // Write data to arduino
    this->WriteString(cmd);

    return;
}

void LN2Controller::WriteValveState(){

    // Sets the valve state on the arduino (1 = open, 0 = close)
    std::string cmd = "s" + std::to_string(CurrentLN2ValveState && !LN2Interlock);

    // Write data to arduino
    this->WriteString(cmd);

    return;
}


void LN2Controller::SendHeartbeat(){

    std::string cmd = "h";

    this->WriteString(cmd);
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
            .values(this->CurrentLN2Valve, this->CurrentLN2ValveState, this->RTDVoltage).execute();

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
