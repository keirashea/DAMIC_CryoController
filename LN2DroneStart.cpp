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

        // Send heartbeat to maintain connection
        LN2Control->SendHeartbeat();

        // Update SQL
        LN2Control->UpdateMysql();

        // Write valve state and state which valve should be operational
        LN2Control->WriteValve();
        LN2Control->WriteValveState();
        LN2Control->ReadRTDVolatge();

        // Implement watchdog -- shuts off liquid nitrogen supplies by forcing state into idle
        if(!LN2Control->WatchdogFuse){
            fflush(stdout);
            printf("\r-------Watchdog fuse blown, system protection active.-----\n");
            LN2Control->CurrentLN2ValveState = 0;
            LN2Control->WriteValveState();
            LN2Control->WatchdogFuse = 0;
            sleep(1);
            continue;
        }

        LN2Control->PrintStatus();

        fflush(stdout);

        sleep(2);
    }

    return 0;
#pragma clang diagnostic pop
}
