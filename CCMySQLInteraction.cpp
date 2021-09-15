//
//  CCMySQLInteraction.cpp
//  CC_R
//
//  Created by Pitam Mitra on 1/6/19.
//  Copyright © 2019 Pitam Mitra. All rights reserved.
//

/*Function to interact with MySQL*/

#include <mysqlx/xdevapi.h>
#include "MysqlCredentials.hpp"
#include "CryoControlSM.hpp"
#include <iostream>

void CryoControlSM::UpdateTargetTemperature(double targetTemp){

    // Connect to server using a connection URL
    mysqlx::Session DDroneSession("localhost", 33060, DMysqlUser, DMysqlPass);
    mysqlx::Schema DDb = DDroneSession.getSchema("DAMICDrone");

    // Accessing an existing table
    mysqlx::Table SendControl = DDb.getTable("ControlParameters");

    // Update the target temperature
    mysqlx::Result SCResult= SendControl.update()
                             .set("TargetTemperature",targetTemp)
                             .where("IDX=1").execute();

    int warnings = SCResult.getWarningsCount();
    if (warnings != 0) std::cout<<"SQL Generated warnings! \n";


    return; 
}

void CryoControlSM::UpdateVars(DataPacket &_thisInteractionData ){

    // Connect to server using a connection URL
    mysqlx::Session DDroneSession("localhost", 33060, DMysqlUser, DMysqlPass);
    mysqlx::Schema DDb = DDroneSession.getSchema("DAMICDrone");

    /*First lets get the control parameters*/
    mysqlx::Table CtrlTable = DDb.getTable("ControlParameters");
    mysqlx::RowResult ControlResult = CtrlTable.select("TargetTemperature", "Kp", "Ki", "Kd", "KpR","KiR", "KdR", "WatchDogFuse", "SRSPowerState", "LN2ValveInterLock")
    .bind("IDX", 1).execute();
    /*The row with the result*/
    mysqlx::Row CtrlRow = ControlResult.fetchOne();

    _thisInteractionData.targetTemp = CtrlRow[0];
    _thisInteractionData.kpA = CtrlRow[1];
    _thisInteractionData.kiA = CtrlRow[2];
    _thisInteractionData.kdA = CtrlRow[3];
    _thisInteractionData.kpR = CtrlRow[4];
    _thisInteractionData.kiR = CtrlRow[5];
    _thisInteractionData.kdR = CtrlRow[6];
    _thisInteractionData.WatchdogFuse = (bool) CtrlRow[7];
    _thisInteractionData.SRSPowerState = (bool) CtrlRow[8];
    _thisInteractionData.LN2ValveInterLock = (bool) CtrlRow[9];


    /* Next - temperature and heater power from the Arduino heater */
    mysqlx::Table ArduinoHeaterTable = DDb.getTable("ArduinoHeaterState");
    mysqlx::RowResult ArdHeatRowResult = ArduinoHeaterTable.select("UNIX_TIMESTAMP(Time)", "TemperatureK1", "TemperatureK2", "HeaterPower")
            .orderBy("Time DESC").limit(1).execute();
    mysqlx::Row ArdHeatRow = ArdHeatRowResult.fetchOne();

    // Parse the mysql data into data struct
    if (ArdHeatRow[0].getType() > 0) _thisInteractionData.LastHeaterArduinoTime = (long) ArdHeatRow[0];
    _thisInteractionData.currentTempK1 = ArdHeatRow[1];
    _thisInteractionData.currentTempK2 = ArdHeatRow[2];

    // Get overflow voltage from LN2 Arduino
    mysqlx::Table LNControllerTable = DDb.getTable("LN2ControllerState");
    mysqlx::RowResult LNControllerRowResult = LNControllerTable.select("UNIX_TIMESTAMP(Time)", "OverflowVoltage").orderBy("Time DESC").limit(1).execute();
    mysqlx::Row LNControllerRow = LNControllerRowResult.fetchOne();

    // Parse LN2
    _thisInteractionData.LastLNArduinoTime = LNControllerRow[0];
    _thisInteractionData.overflowVoltage = LNControllerRow[1];

    // Get Cup RTD info
    mysqlx::Table CupTempTable = DDb.getTable("ArduinoCupTemp");
    mysqlx::RowResult CupTempRowResult = CupTempTable.select("UNIX_TIMESTAMP(TimeS)", "R0","R1","Temp0","Temp1").orderBy("IDX DESC").limit(1).execute();
    mysqlx::Row CupTempRow = CupTempRowResult.fetchOne();

    // Parse Cup RTD
    _thisInteractionData.CupTempR0 = CupTempRow[1];
    _thisInteractionData.CupTempR1 = CupTempRow[2];
    _thisInteractionData.CupTempT0 = CupTempRow[3];
    _thisInteractionData.CupTempT1 = CupTempRow[4];

    /*Now update the monitoring table*/

    // Accessing an existing table
    mysqlx::Table SMStats = DDb.getTable("SMState");
    unsigned int warnings;

    // Insert SQL Table data
    //mysqlx::Result SMStatsResult= SMStats.insert("PID", "SystemState", "ShouldBeState", "TimeInCurrentLNState")
    //.values(this->ThisRunPIDValue, (int)this->CurrentFSMState, (int)this->ShouldBeFSMState, this->TimeInCurrentLNState).execute();
    //warnings=SMStatsResult.getWarningsCount();

    // Insert SQL Table data
    mysqlx::Result SMStatsResult= SMStats.insert("PID", "SystemState", "ShouldBeState", "TimeInCurrentLNState")
    .values(this->ThisRunPIDValue, (int)this->CurrentFSMState, (int)this->ShouldBeFSMState, this->TimeInCurrentLNState).execute();
    warnings=SMStatsResult.getWarningsCount();


    // Accessing an existing table
    mysqlx::Table SendControl = DDb.getTable("ControlParameters");

    // Insert SQL Table data
    mysqlx::Result SCResult= SendControl.update()
                                        .set("HeaterPower",this->ThisRunHeaterPower)
                                        .set("SMState", (int)this->CurrentFSMState)
                                        .where("IDX=1").execute();
    warnings+=SCResult.getWarningsCount();

    //Update the SRSPowerState if needed;
    if (_thisInteractionData.SRSPowerState != this->ComputedSRSPowerState){
        SCResult= SendControl.update().set("SRSPowerState",this->ComputedSRSPowerState).where("IDX=1").execute();
        warnings+=SCResult.getWarningsCount();

        //Debug
        printf("SRS Power state switched %d\n",this->ComputedSRSPowerState);

    }

    //Update the LN2Valve if needed;
    SCResult= SendControl.update()
                             .set("LN2ValveInterLock",this->LN2Interlock)
                             .set("LN2ValveState", this->ThisRunValveState)
                             .where("IDX=1").execute();

    warnings+=SCResult.getWarningsCount();



    if (warnings != 0) std::cout<<"SQL Generated warnings! \n";

    DDroneSession.close();

}

