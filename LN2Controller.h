//
// Created by apiers on 3/16/20.
//

#ifndef DAMICMCRYOCTRL_LN2CONTROLLER_H
#define DAMICMCRYOCTRL_LN2CONTROLLER_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>

#include "UDPClient.hpp"
#include "UtilityFunctions.hpp"


class LN2Controller: public UDPClient {

public:
<<<<<<< HEAD
    LN2Controller(std::string);
=======
    LN2Controller();
>>>>>>> c8e0ed6bc3f91db69b7dbf5ca32785287c6587c8
    ~LN2Controller();

    // Get functions
    void ReadRTDVolatge();

    // Set functions
    void WriteValve();
    void WriteValveState();

    void SendHeartbeat();

    // Other functions
    void UpdateMysql(void );
    void PrintStatus();

    bool WatchdogFuse;
    int CurrentLN2Valve;
    int CurrentLN2ValveState;
    float RTDVoltage;
    bool LN2Interlock;


    std::string SQLStatusMsg;


};


#endif //DAMICMCRYOCTRL_LN2CONTROLLER_H
