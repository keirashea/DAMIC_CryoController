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
//    auto * ArdHeat = new ArduinoHeater("/dev/tty0");
    auto * ArdHeat = new ArduinoHeater();
    sleep(1);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {

        // Generate some random data for testing
        float randTemp = 290 + static_cast<float> (rand()) / static_cast<float> (RAND_MAX / 3);
        int randPower = rand() % 1023;
        ArdHeat->currentTemperatureK = randTemp;
        ArdHeat->currentPower = randPower;
//        ArdHeat->ReadPower();
//        ArdHeat->ReadTemperatureK();
        ArdHeat->UpdateMysql();

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

        sleep(2);
    }
#pragma clang diagnostic pop


}