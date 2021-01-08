//
// Created by apiers on 3/16/20.
//

#ifndef DAMICMCRYOCTRL_LN2CONTROLLER_H
#define DAMICMCRYOCTRL_LN2CONTROLLER_H

// Includes
#include <iostream>
#include "SerialDeviceT.hpp"

#define OVERFLOW_THRESHOLD 2.5

class LN2Controller: public SerialDevice {

public:
    LN2Controller();
    LN2Controller(std::string);
    ~LN2Controller();

    // Get functions
    void ReadValveState();
    void ReadTimeInCurrentState();
    void ReadTimeBetweenFillState();
    void ReadOverflowVoltage();

    // Set functions
    void WriteSMState(int smState);
    void WriteCurrentTemperature(float temperature);

    // Other functions
    void UpdateMysql(void );

    bool WatchdogFuse;
    bool valveState;
    bool isOverflow;
    int smState;
    int timeBetweenFillState;
    float currentTemperature;
    unsigned long timeInCurrentState;
    float overflowVoltage;

    std::string SQLStatusMsg;


};


#endif //DAMICMCRYOCTRL_LN2CONTROLLER_H
