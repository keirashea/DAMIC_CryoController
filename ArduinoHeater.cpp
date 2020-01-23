// Serial interaction with the Arudino heater and temperature sensor
// Interfaces device to database to store data
// Created by apiers on 1/22/20.
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

#include "SerialDeviceT.hpp"
#include "ArduinoHeater.h"
#include "MysqlCredentials.hpp"


#include <mysqlx/xdevapi.h>

ArduinoHeater::ArduinoHeater(){
    this->WatchdogFuse = 1;
    this->setPower = 0;
    this->currentTemperatureK = -1;
    this->currentPower = -1;

    printf("Dummy instance of class. No serial communication available");

}

ArduinoHeater::ArduinoHeater(std::string SerialPort) : SerialDevice(SerialPort) {

    /* Set Baud Rate */
    cfsetospeed (&this->tty, (speed_t)B9600);
    cfsetispeed (&this->tty, (speed_t)B9600);

    /* Setting other Port Stuff */
    tty.c_cflag     |=  PARENB;
    tty.c_cflag     |= PARODD;
    tty.c_cflag     &=  ~CSTOPB;
    tty.c_cflag     &=  ~CSIZE;
    tty.c_cflag     |=  CS7;
    tty.c_cflag |= IXON ;
    tty.c_cflag     &=  ~CRTSCTS;           // no flow control

    /* Flush Port, then applies attributes */
    tcflush( USB, TCIFLUSH );
    if ( tcsetattr ( USB, TCSANOW, &tty ) != 0)
    {
        std::cout << "Error " << errno << " from tcsetattr" << std::endl;
    }


    this->WatchdogFuse = 1;
    this->setPower = 0;
    this->currentTemperatureK = -1;
    this->currentPower = -1;

    printf("Arduino heater is now ready to accept instructions.\n");
}

ArduinoHeater::~ArduinoHeater() {
    close(USB);
}

void ArduinoHeater::ReadPower() {
    // Reads Power being sent to the Arduino heater

    std::string ArdP_String;
    std::string ArdCmd = "P\r\n";


    this->WriteString(ArdCmd);
    ArdP_String = this->ReadLine();


    try{
        this->currentPower = std::stof(ArdP_String);
    } catch (...) {
        printf("Error in ReadPower. Continuing...\n ");
    }
}

void ArduinoHeater::ReadTemperatureK() {
    // Reads the temperature in K
    std::string ArdT_String;
    std::string ArdCmd = "T\r\n";


    this->WriteString(ArdCmd);
    ArdT_String = this->ReadLine();


    try{
        this->currentTemperatureK = std::stof(ArdT_String) + 273.;
    } catch (...) {
        printf("Error in ReadTemperatureK. Continuing...\n ");
    }

}

void ArduinoHeater::SetPower(int newPower) {

    if (newPower < ARD_MINIMUM_POWER) newPower = ARD_MINIMUM_POWER;
    if (newPower > ARD_MAXIMUM_POWER) newPower = ARD_MAXIMUM_POWER;

    std::string ArdCmd = "S " + std::to_string(newPower) + "\r\n";

    this->WriteString(ArdCmd);
    this->setPower = newPower;

}

void ArduinoHeater::UpdateMysql(void) {

    // Connect to  Database
    mysqlx::Session DDroneSession("localhost", 33060, DMysqlUser, DMysqlPass);
    mysqlx::Schema DDatabase = DDroneSession.getSchema("DAMICDrone");

    // Get control parameter table
    mysqlx::Table CtrlParameterTable = DDatabase.getTable("ControlParameters");
    mysqlx::RowResult CtrlRowResult = CtrlParameterTable.select("HeaterPower", "WatchDogFuse")
            .bind("IDX", 1).execute();
    mysqlx::Row CtrlRow = CtrlRowResult.fetchOne();

    this->setPower = CtrlRow[0];
    this->_cWatchdogFuse = CtrlRow[1];

    // Get the Arduino Heater table to update and insert values
    mysqlx::Table ArduinoHeaterState = DDatabase.getTable("ArduinoHeaterState");
    mysqlx::Result ArdHeaterRes = ArduinoHeaterState.insert("HeaterPower", "HeaterSetPower", "TemperatureK1", "WatchDogState")
            .values(this->currentPower, this->setPower, this->currentTemperatureK, this->WatchdogFuse).execute();

    // Check to see if values were inserted properly
    unsigned int warnings = ArdHeaterRes.getWarningsCount();
    (warnings == 0) ? this->SQLStatusMsg = "OK" : this->SQLStatusMsg = "WARN!\n";

    // Close the database connection
    DDroneSession.close();


}