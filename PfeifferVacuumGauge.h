// Serial Control of the Pfeiffer TPG361 Pressure Gauge
// Created by Alex Piers on 2/4/20.
//

#ifndef DAMICMCRYOCTRL_PFEIFFERVACUUMGAUGE_H
#define DAMICMCRYOCTRL_PFEIFFERVACUUMGAUGE_H

/*Includes*/
#include <iostream>
#include "SerialDeviceT.hpp"
#include <sstream>


class PfeifferVacuumGauge : public SerialDevice {



public:

    // Constructors
    PfeifferVacuumGauge();

    PfeifferVacuumGauge(const std::string &string);

    virtual ~PfeifferVacuumGauge();



    // Read functions
    void ReadPressure();
    void ReadGaugeError();

    // Set functions

    // Other functions
    void UpdateMysql(void );

    // Getters
    float getCurrentPressure() const;
    bool isWatchdogFuse() const;
    const std::string &getSqlStatusMsg() const;

    int getGaugeError() const;

    void setGaugeError(int gaugeError);

    // Setters
    void setWatchdogFuse(bool watchdogFuse);

    int getMeasurementStatus() const;

    void setMeasurementStatus(int measurementStatus);

private:

    float currentPressure;
    bool WatchdogFuse;
    int measurementStatus;
    int gaugeError;
    std::string SQLStatusMsg;

    const char enquire = 5;
    const char acknowledge = 6;

    // Maximum iterations for sending commands
    const int MAX_NUMBER_OF_REQUEST = 5;
};


#endif //DAMICMCRYOCTRL_PFEIFFERVACUUMGAUGE_H
