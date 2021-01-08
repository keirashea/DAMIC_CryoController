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
    this->valveState = 0;
    this->timeBetweenFillState = 0;
    this->overflowVoltage = 0;
    this->timeInCurrentState = 0;
    this->isOverflow = 0;

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
        this->valveState = 0;
        this->timeBetweenFillState = 0;
        this->overflowVoltage = 0;
        this->timeInCurrentState = 0;
        this->isOverflow = 0;

        printf("Liquid Nitrogen Controller is now ready to accept instructions.\n");
}

LN2Controller::~LN2Controller(){
    close(USB);
};

// Get functions
void LN2Controller::ReadValveState(){
    // Gets the valve state from the arduino (1 = open, 0 = close)
    std::string ArdLn2_String;
    std::string cmd = "v";

    this->WriteString(cmd);
    ArdLn2_String = this->ReadLine();

    try {
        this->valveState = std::stoi(ArdLn2_String);
    } catch(...){
        printf("Error in GetValveState. Continuing...\n");
    }
}

void LN2Controller::ReadTimeInCurrentState(){
    // Gets the time in the current state in seconds
    std::string ArdLn2_String;
    std::string cmd = "c";

    this->WriteString(cmd);
    ArdLn2_String = this->ReadLine();

    try {
        this->timeInCurrentState = std::stoi(ArdLn2_String) / 1000;
    } catch(...){
        printf("Error in ReadTimeInCurrentState. Continuing...\n");
    }
}
void LN2Controller::ReadTimeBetweenFillState(){
    std::string ArdLn2_String;
    std::string cmd = "b";

    this->WriteString(cmd);
    ArdLn2_String = this->ReadLine();

    try {
        this->timeBetweenFillState = std::stoi(ArdLn2_String);
    } catch(...){
        printf("Error in GetTimeBetweenFillState. Continuing...\n");
    }
}
void LN2Controller::ReadOverflowVoltage(){
    std::string ArdLn2_String;
    std::string cmd = "o";

    this->WriteString(cmd);
    ArdLn2_String = this->ReadLine();

    try {
        this->overflowVoltage = std::stof(ArdLn2_String);
    } catch(...){
        printf("Error in GetTimeBetweenFillState. Continuing...\n");
    }

    this->isOverflow = overflowVoltage > OVERFLOW_THRESHOLD;
}

// Set functions
void LN2Controller::WriteSMState(int smState){

    std::string ArdLn2_String;
    std::string cmd = "s" + std::to_string(smState);

    // Write to arduino
    this->WriteString(cmd);
    // this->
}
void LN2Controller::WriteCurrentTemperature(float temperature){

    std::string ArdLn2_String;
    std::string cmd = "t" + std::to_string(temperature);

    // Write to arduino
    this->WriteString(cmd);
}

// Other functions
void LN2Controller::UpdateMysql(void ){


    // Connect to  Database
    mysqlx::Session DDroneSession("localhost", 33060, DMysqlUser, DMysqlPass);
    mysqlx::Schema DDatabase = DDroneSession.getSchema("DAMICDrone");

    // Get SM state
    // mysqlx::Table SMParameterTable = DDatabase.getTable("SMState");
    // mysqlx::RowResult SMRowResult = SMParameterTable.select("SystemState")
    //         .orderBy("IDX DESC").limit(1).execute();

    // mysqlx::Row SMRow = SMRowResult.fetchOne();
    // this->smState = SMRow[0];

    // Get temperature from heater database
    mysqlx::Table HeaterParameterTable = DDatabase.getTable("ArduinoHeaterState");
    mysqlx::RowResult HeaterRowResult = HeaterParameterTable.select("TemperatureK2", "TemperatureK1")
            .orderBy("IDX DESC").limit(1).execute();
    mysqlx::Row HeaterRow = HeaterRowResult.fetchOne();
    this->currentTemperature = (double) HeaterRow[0] > 0 ? (double)HeaterRow[0] : (double)HeaterRow[1];

    // Get watchdog fuse from control parameters
    mysqlx::Table CtrlParameterTable = DDatabase.getTable("ControlParameters");
    mysqlx::RowResult CtrlRowResult = CtrlParameterTable.select("WatchDogFuse", "SMState")
            .bind("IDX", 1).execute();
    mysqlx::Row CtrlRow = CtrlRowResult.fetchOne();
    this->WatchdogFuse = (bool) CtrlRow[0];
    this->smState = CtrlRow[1];

    // Get the LN2 table to update and insert values
    mysqlx::Table LN2ControllerTable = DDatabase.getTable("LN2ControllerState");
    mysqlx::Result LN2ControllerResult = LN2ControllerTable.insert("ValveState", "TimeBetweenFillState", "TimeInCurrentState", "SMState", "CurrentTemperature", "OverflowVoltage")
            .values(this->valveState, this->timeBetweenFillState, this->timeInCurrentState, this->smState, this->currentTemperature, this->overflowVoltage).execute();

    // Check to see if values were inserted properly
    unsigned int warnings = LN2ControllerResult.getWarningsCount();
    (warnings == 0) ? this->SQLStatusMsg = "OK" : this->SQLStatusMsg = "WARN!\n";

    // Close the database connection
    DDroneSession.close();



    return;
}
