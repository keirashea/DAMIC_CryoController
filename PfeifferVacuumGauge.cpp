// Serial Control of the Pfeiffer TPG361 Pressure Gauge
// Created by Alex Piers on 2/4/20.
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
#include <exception>

#include "SerialDeviceT.hpp"
#include "PfeifferVacuumGauge.h"
#include "MysqlCredentials.hpp"


#include <mysqlx/xdevapi.h>

PfeifferVacuumGauge::PfeifferVacuumGauge() {
    this->WatchdogFuse = 1;
    this->currentPressure = -1;
    this->measurementStatus = -1;
    this->gaugeError = -1;

    printf("Dummy instance of class. No serial communication available\n");
}

PfeifferVacuumGauge::PfeifferVacuumGauge(const std::string &SerialPort) : SerialDevice(SerialPort) {

    /* Set Baud Rate */
    cfsetospeed (&this->tty, (speed_t)B9600);
    cfsetispeed (&this->tty, (speed_t)B9600);

    /* Setting other port information. See gauge documentations for why we formatted the way we did */
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
//    tty.c_lflag |= (ICANON | ECHO | ECHOE);


    /* Flush Port, then applies attributes */
    tcflush( USB, TCIFLUSH );
    if ( tcsetattr ( USB, TCSANOW, &tty ) != 0)
    {
        std::cout << "Error " << errno << " from tcsetattr" << std::endl;
    }

    this->WatchdogFuse = 1;
    this->currentPressure = -1;
    this->measurementStatus = -1;
    this->gaugeError = -1;
    printf("Pfeiffer Vacuum Gauge is now ready to accept instructions.\n");
}

PfeifferVacuumGauge::~PfeifferVacuumGauge() {
    close(USB);
}

void PfeifferVacuumGauge::ReadPressure() {
    // Reads the pressure from the gaus

    std::string Pfeiff_String;
    std::string Pfeiff_Cmd = "PR1\r";

    // Send Command
    this->WriteString(Pfeiff_Cmd);
    Pfeiff_String = this->ReadLine();

    // Check acknowledge. Sometimes need to resend command
    if(Pfeiff_String.at(0) != acknowledge){
        printf("Unable to send command %s to Gauge.\n", Pfeiff_Cmd.c_str());
        return;
    }

    // Read the next line
    this->WriteChar(&enquire, 1);
    Pfeiff_String = this->ReadLine();

    // Parse pressure line
    std::stringstream parseresult(Pfeiff_String);
//    parseresult.str(Pfeiff_String);
    try {
        std::string Pfeiff_SubString;
        std::getline(parseresult, Pfeiff_SubString, ',');

        // Address the status of the sensor
        setMeasurementStatus(stoi(Pfeiff_SubString));
        if( measurementStatus != 0){
            printf("Pressure measurement error status %i. Continuing \n", measurementStatus);
            return;
        }

        // Get the pressure
        std::getline(parseresult, Pfeiff_SubString, ',');
        currentPressure = std::stof(Pfeiff_SubString);

    }catch(std::exception& e){
        printf("Error in ReadPressure. Continuing...\n ");
    }

}

void PfeifferVacuumGauge::ReadGaugeError() {
    std::string Pfeiff_String;
    std::string Pfeiff_Cmd = "ERR\r\n";

    // Send Command
    this->WriteString(Pfeiff_Cmd);
    Pfeiff_String = this->ReadLine();

    // Check acknowledge
    if(Pfeiff_String.at(0) != acknowledge){
        printf("Unable to send command %s to Gauge\n", Pfeiff_Cmd.c_str());
        return;
    }

    // Read the next line
    this->WriteChar(&enquire, 1);
    Pfeiff_String = this->ReadLine();

    try {
        gaugeError = stoi(Pfeiff_String);
    }catch (...){
        printf("Error in ReadPressure. Continuing...\n ");
    }

}

void PfeifferVacuumGauge::UpdateMysql(void) {

    // Connect to  Database
    mysqlx::Session DDroneSession("localhost", 33060, DMysqlUser, DMysqlPass);
    mysqlx::Schema DDatabase = DDroneSession.getSchema("DAMICDrone");

    // Get control parameter table
    mysqlx::Table CtrlParameterTable = DDatabase.getTable("ControlParameters");
    mysqlx::RowResult CtrlRowResult = CtrlParameterTable.select("WatchDogFuse")
            .bind("IDX", 1).execute();
    mysqlx::Row CtrlRow = CtrlRowResult.fetchOne();

    this->WatchdogFuse = static_cast<bool>( CtrlRow[0] );

    // Get the pressure table
    mysqlx::Table PfeifferStateTable = DDatabase.getTable("VacuumState");
    mysqlx::Result PfeifferStateRes = PfeifferStateTable.insert("Pressure", "GaugeMeasurementStatus", "GaugeErrorStatus")
            .values(getCurrentPressure(), getMeasurementStatus(), getGaugeError()).execute();

    // Check to see if values were inserted properly
    unsigned int warnings = PfeifferStateRes.getWarningsCount();
    (warnings == 0) ? SQLStatusMsg = "OK" : SQLStatusMsg = "WARN!\n";

    // Close the database connection
    DDroneSession.close();
}

float PfeifferVacuumGauge::getCurrentPressure() const {
    return currentPressure;
}

bool PfeifferVacuumGauge::isWatchdogFuse() const {
    return WatchdogFuse;
}

const std::string &PfeifferVacuumGauge::getSqlStatusMsg() const {
    return SQLStatusMsg;
}

void PfeifferVacuumGauge::setWatchdogFuse(bool watchdogFuse) {
    WatchdogFuse = watchdogFuse;
}

int PfeifferVacuumGauge::getMeasurementStatus() const {
    return measurementStatus;
}

void PfeifferVacuumGauge::setMeasurementStatus(int measurementStatus) {
    PfeifferVacuumGauge::measurementStatus = measurementStatus;
}


int PfeifferVacuumGauge::getGaugeError() const {
    return gaugeError;
}

void PfeifferVacuumGauge::setGaugeError(int gaugeError) {
    PfeifferVacuumGauge::gaugeError = gaugeError;
}
