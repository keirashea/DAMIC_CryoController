//
// Created by apiers on 3/16/20.
//

// Standard Headers
#include <iostream>
#include <vector>
#include <unistd.h>

// Custom Headers
#include "LN2Controller.h"

int main(int argc, char ** argv){

    // Create new serial object
    auto * LN2Control = new LN2Controller("/dev/ArduinoLN2"); // may need to change the name
//    auto * LN2Control = new LN2Controller(); // Dummy class
    sleep(1);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {

        // Read current parameters
        LN2Control->ReadValveState();
        LN2Control->ReadTimeInCurrentState();
        LN2Control->ReadTimeBetweenFillState();
        LN2Control->ReadOverflowVoltage();

        // Update SQL
        LN2Control->UpdateMysql();

        // Send database dependent parameters
        LN2Control->WriteSMState(LN2Control->smState);
        LN2Control->WriteCurrentTemperature(LN2Control->currentTemperature);

        // Implement watchdog -- shuts off liquid nitrogen supplies by forcing state into idle
        if(!LN2Control->WatchdogFuse){
            fflush(stdout);
            printf("\r-------Watchdog fuse blown, system protection active.-----\n");
            LN2Control->WriteSMState(0);
            LN2Control->WatchdogFuse = 0;
            sleep(1);
            continue;
        }

        fflush(stdout);
        printf("\rLiquid Nitrogen Control | Valve State: %i, Time in State (s): %lu, Time Between Fills State: %i, Overflow Pin (V): %0.2f, Is Overflow? %i, SMState: %i, Temp (K): %0.2f",
                LN2Control->valveState, LN2Control->timeInCurrentState, LN2Control->timeBetweenFillState, LN2Control->overflowVoltage, LN2Control->isOverflow, LN2Control->smState, LN2Control->currentTemperature);

        sleep(2);
    }




    return 0;
#pragma clang diagnostic pop
}

