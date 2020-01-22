//
// Created by apiers on 1/22/20.
//

/*Std headers*/
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <unistd.h>


/*Custom Headers*/
#include "SerialDeviceT.hpp"
#include "ArduinoHeater.h"
#include <mysqlx/xdevapi.h>


int main(int argc, char** argv)
{
    // Create Serial Object
    ArduinoHeater * ArdHeat = new ArduinoHeater("/dev/tty0");
    sleep(1);

    while (true) {

        ArdHeat->ReadPower();
        ArdHeat->ReadTemperatureK();

        if (ArdHeat->_cWatchdogFuse != 1){
            fflush(stdout);
            printf("\r-------Watchdog fuse blown, system protection active.-----\n");
            ArdHeat->SetPower(0);
            ArdHeat->WatchdogFuse = 0;
            sleep(1);
            continue;
        }

        fflush(stdout);
        printf("\rArduino Heater | Power: %0.2f,  Temp (K): %0.2f",
                ArdHeat->currentPower / ArdHeat->maximumPower, ArdHeat->currentTemperatureK);
    }


}