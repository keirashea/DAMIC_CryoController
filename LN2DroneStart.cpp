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
<<<<<<< HEAD
	std::string server_ip = "";
    auto * LN2Control = new LN2Controller(server_ip); // may need to change the name
//    auto * LN2Control = new LN2Controller(); // Dummy class
=======
    auto * LN2Control = new LN2Controller();
>>>>>>> c8e0ed6bc3f91db69b7dbf5ca32785287c6587c8
    sleep(1);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {

        // Send heartbeat to maintain connection
        LN2Control->SendHeartbeat();
        LN2Control->ReadRTDVolatge();

        // Update SQL
        LN2Control->UpdateMysql();

        // Write valve state and state which valve should be operational
        LN2Control->WriteValve();
        LN2Control->WriteValveState();


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
