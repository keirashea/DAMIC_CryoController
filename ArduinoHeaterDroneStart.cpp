//
// Created by apiers on 1/22/20.
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
#include "ArduinoHeater.h"
#include <mysqlx/xdevapi.h>


int main(int argc, char** argv)
{
    // Create Serial Object
    auto * ArdHeat = new ArduinoHeater("/dev/ArduinoHeater");
//    auto * ArdHeat = new ArduinoHeater();
    sleep(1);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {

/*      // Generate some random data for testing
        float randTemp = 290 + static_cast<float> (rand()) / static_cast<float> (RAND_MAX / 3);
        int randPower = rand() % 1023;
        ArdHeat->currentTemperatureK1 = randTemp;
        ArdHeat->currentTemperatureK2 = randTemp - 1. + static_cast<float> (rand()) / static_cast<float> (RAND_MAX / 2);
        ArdHeat->currentPower = static_cast<int> (rand()) % ARD_MAXIMUM_POWER;
*/

        // Read current parameters
        ArdHeat->ReadPower();
        ArdHeat->ReadTemperatureK();

        ArdHeat->SendHeartbeat();
        ArdHeat->UpdateMysql();

        // Set power based on ArdHeat->setPower
        ArdHeat->SetPower(ArdHeat->setPower);

        if (ArdHeat->_cWatchdogFuse != 1){
            fflush(stdout);
            printf("\r-------Watchdog fuse blown, system protection active.-----\n");
            ArdHeat->SetPower(0);
            ArdHeat->WatchdogFuse = 0;
            sleep(1);
            continue;
        }




        fflush(stdout);
        printf("\rArduino Heater | Power: %0.2f,  Temp1 (K): %0.2f,  Temp2 (K): %0.2f",
                ArdHeat->currentPower / ARD_MAXIMUM_POWER, ArdHeat->currentTemperatureK1, ArdHeat->currentTemperatureK2);

        sleep(2);
    }
#pragma clang diagnostic pop


}
