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

const double resistance = 25; // ohms
const double voltageMaximum = 50; // volts

class ArduinoHeater: public SerialDevice {
public:
    ArduinoHeater();
    ArduinoHeater(std::string);
    ~ArduinoHeater();

    // Read functions
    void ReadPower();
    void ReadTemperatureK();

    // Set functions
    void SetPowerW(float power);
    void SendHeartbeat();

    // Other functions
    void UpdateMysql(void );
    int ConvertPowerToArduinoUnits(float power);
    float ConvertArduinoUnitsToPower(int power);

    int currentPower;
    float currentTemperatureK1;
    float currentTemperatureK2;
    float setPowerWatts;
    int setPower;

    bool WatchdogFuse;
    std::string SQLStatusMsg;
    int _cHeaterMode, _cWatchdogFuse;


};


#endif //ARDUINOHEATER_H
