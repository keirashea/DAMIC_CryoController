/* ***********************************************
 * DAMICM CCD Temperature controller
 *
 *
 * *********************************************** */

/*Std headers*/
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <unistd.h>


/*Custom Headers*/
#include "SerialDeviceT.hpp"
#include "ArduinoCupTemp.hpp"
#include <mysqlx/xdevapi.h>




int main( int argc, char** argv )
{

    ArdCupTemp *ArdTemp = new ArdCupTemp("/dev/ttyACM2");
    sleep(1);

    while (true){

        ArdTemp->ReadResistor();
        //ArdTemp->ReadMode();
        
        
        //ArdTemp->UpdateMysql();

        if (ArdTemp->_cWatchdogFuse != 1){
            fflush(stdout);
            printf("\r-------Watchdog fuse blown, system protection active.-----\n");
            //ArdTemp->TurnONOFF(0);
            ArdTemp->WatchdogFuse = 0;
            sleep(1);
            continue;
        }

        fflush(stdout);
        //printf ("\rLakeShore | PW: %.02f, Temp: %.2f K,  Mode (Set): %1d(%1d),  Watchdog Fuse: %.02d, MySQL: %s",
        //        LSHeater->currentPW, LSHeater->currentTempK, LSHeater->currentMode, LSHeater->setMode, LSHeater->WatchdogFuse, LSHeater->SQLStatusMsg.c_str());

        sleep(1);


    }
    printf ("\n");




    delete ArdTemp;


    return 0;
}

