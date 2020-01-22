//
// Created by apiers on 1/22/20.
//

#ifndef ARDUINOHEATER_H
#define ARDUINOHEATER_H

/*Includes*/
#include <iostream>
#include "SerialDeviceT.hpp"


class ArduinoHeater: public SerialDevice {
public:
    ArduinoHeater(std::string);
    ~ArduinoHeater();

    // Read functions
    void ReadPower();
    void ReadTemperatureK();

    // Set functions
    void SetPower(int power);

    // Other functions
    void UpdateMysql(void );

    float currentPower;
    float currentTemperatureK;
    int setPower;

    bool WatchdogFuse;
    std::string SQLStatusMsg;
    int _cHeaterMode, _cWatchdogFuse;

    // Limits on the capabilities of the arduino heaters
    const int minimumPower = 0;
    const int maximumPower = 1023;

};


#endif //ARDUINOHEATER_H
