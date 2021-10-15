//
// Created by apiers on 3/16/20.
//

#ifndef DAMICMCRYOCTRL_LN2CONTROLLER_H
#define DAMICMCRYOCTRL_LN2CONTROLLER_H

// Includes
#include <iostream>
#include <ctime>
#include "UtilityFunctions.hpp"
#include "SerialDeviceT.hpp"


class LN2Controller: public SerialDevice {

public:
    LN2Controller();
    LN2Controller(std::string);
    ~LN2Controller();

    // Get functions
    void ReadOverflowVoltage();

    // Set functions
    void WriteValveState();
    void SendHeartbeat();

    // Other functions
    void UpdateMysql(void );
    void PrintStatus();

    bool WatchdogFuse;
    int ValveState;
    float overflowVoltage;
    bool LN2Interlock;

    std::string SQLStatusMsg;


};


#endif //DAMICMCRYOCTRL_LN2CONTROLLER_H
