// Starts the monitoring of the Pfeiffer pressure (pump/gauge) products
// Created by Alex Piers on 2/4/20.
//

/*Std headers*/
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <unistd.h>
#include <ctime>
#include <cstdlib>

/*Custom Headers*/
#include "SerialDeviceT.hpp"
#include <mysqlx/xdevapi.h>
#include "PfeifferVacuumGauge.h"

int main(int argc, char** argv)
{
    // Create Serial Object
    auto * PfeifferGauge = new PfeifferVacuumGauge("/dev/ttyUSB0");
//    auto * ArdHeat = new ArduinoHeater();
    sleep(1);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {

        // Read current parameters
        PfeifferGauge->ReadPressure();
        PfeifferGauge->ReadGaugeError();
        PfeifferGauge->UpdateMysql();

        if (PfeifferGauge->isWatchdogFuse() != 1){
            fflush(stdout);
            printf("\r-------Watchdog fuse blown, system protection active.-----\n");
            PfeifferGauge->setWatchdogFuse(0);
            sleep(1);
            continue;
        }



        fflush(stdout);
        printf("\rPfeiffer Tools | Gauge Pressure (mbar): %0.2e,  Measurement Status: %i, Gauge Error Status: %i",
               PfeifferGauge->getCurrentPressure(), PfeifferGauge->getMeasurementStatus(), PfeifferGauge->getGaugeError());

        sleep(1);
    }
#pragma clang diagnostic pop


}
