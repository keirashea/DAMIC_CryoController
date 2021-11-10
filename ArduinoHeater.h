//
// Created by apiers on 1/22/20.
//

#ifndef ARDUINOHEATER_H
#define ARDUINOHEATER_H

/*Includes*/
#include <iostream>
#include "SerialDeviceT.hpp"

#define ARD_MAXIMUM_POWER 1023
#define ARD_MINIMUM_POWER 0

class ArduinoHeater: public SerialDevice {
public:
    ArduinoHeater();
    ArduinoHeater(std::string);
    ~ArduinoHeater();

    // Read functions
    void ReadPower();
    void ReadTemperatureK();

    // Set functions
    void SetPower(int power);
    void SendHeartbeat();

    // Other functions
    void UpdateMysql(void );

    float currentPower;
    float currentTemperatureK1;
    float currentTemperatureK2;
    int setPower;

    bool WatchdogFuse;
    std::string SQLStatusMsg;
    int _cHeaterMode, _cWatchdogFuse;


};


#endif //ARDUINOHEATER_H
