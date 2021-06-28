/* **************************************************************************************
 * Code containing the SerialDevice class that will have the definitions we will need
 * for talking to the Cryocooler and Heater
 *
 * by Pitam Mitra 2018 for DAMIC-M
 * **************************************************************************************/

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
#include "ArduinoCupTemp.hpp"
#include "MysqlCredentials.hpp"


#include <mysqlx/xdevapi.h>



ArdCupTemp::ArdCupTemp(std::string SerialPort) : SerialDevice(SerialPort){


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


    //this->RTDResistance (8,0);

    printf("Arduino Uno with RTDs is now ready to accept instructions.\n");

}


ArdCupTemp::~ArdCupTemp()
{
    close(USB);
}



void ArdCupTemp::ReadResistor()
{
    std::string Ard_String;
    std::string ArCmd = "rtd,0*\r\n";


    //this->WriteString(ArCmd);
    Ard_String = this->ReadLine();

    std::cout<<Ard_String<<"\n";
    
    try{
        this->RTDResistance[0] = std::stof(Ard_String);
    } catch (...) {
        printf("Error in ReadPower. Continuing...\n ");
    }
    
}

void ArdCupTemp::CalculateTemp()
{
    std::string Ard_String;
    std::string ArCmd = "KRDG?\r\n";


    this->WriteString(ArCmd);
    Ard_String = this->ReadLine();

    //std::cout<<this->Ard_String<<"\n";
    
    try{
        //this->currentTempK = std::stof(Ard_String);
    } catch (...) {
        //printf("Error in ReadPower. Continuing...\n ");
    }
    
}


void ArdCupTemp::UpdateMysql(void){

    int _cWatchdogFuse;

    // Connect to server using a connection URL
    mysqlx::Session DDroneSession("localhost", 33060, DMysqlUser, DMysqlPass);
    mysqlx::Schema DDb = DDroneSession.getSchema("DAMICDrone");

    /*First lets get the control parameters*/
    mysqlx::Table CtrlTable = DDb.getTable("ControlParameters");
    mysqlx::RowResult ControlResult = CtrlTable.select("WatchdogFuse")
      .bind("IDX", 1).execute();
    /*The row with the result*/
    mysqlx::Row CtrlRow = ControlResult.fetchOne();
    
    this->_cWatchdogFuse = CtrlRow[0];



    /*Now update the monitoring table*/

    // Accessing an existing table
    mysqlx::Table LSHStats = DDb.getTable("CupTemperatures");

    // Insert SQL Table data

    mysqlx::Result LSHResult= LSHStats.insert("TopR", "MidR", "BotR", "WatchdogState")
           .values(this->RTDResistance[0], this->RTDResistance[1], this->RTDResistance[2], WatchdogFuse).execute();

    unsigned int warnings;
    warnings=LSHResult.getWarningsCount();

    if (warnings == 0) this->SQLStatusMsg = "OK";
    else (SQLStatusMsg = "WARN!\n");


    DDroneSession.close();

}


