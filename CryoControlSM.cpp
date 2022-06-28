//
//  CryoControlSM.cpp
//  CC_R
//
//  Created by Pitam Mitra on 1/6/19.
//  Copyright © 2019 Pitam Mitra. All rights reserved.
//
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <unistd.h>
#include <mysqlx/xdevapi.h>
#include <cmath>
#include <chrono>

#include "CryoControlSM.hpp"
#include "ArduinoHeater.h"
#include "PID_v1.h"

CryoControlSM::CryoControlSM(void)
{

    /*The jump table to different states - implementation*/
    this->STFnTable = {
        {ST_Idle, &CryoControlSM::Idle},
        {ST_CoolDown, &CryoControlSM::CoolDown},
        {ST_Warmup, &CryoControlSM::Warmup},
        {ST_MaintainCold, &CryoControlSM::MaintainCold},
        {ST_MaintainWarm, &CryoControlSM::MaintainWarm},
        {ST_Fault, &CryoControlSM::Fault}};

    /*Current starting state for FSM is idle. Should be state is also idle*/
    CurrentFSMState = ST_Idle;
    ShouldBeFSMState = ST_Idle;
    this->CryoStateFn = this->STFnTable[ShouldBeFSMState];

    CurrentTemperature = 0;
    LastTemperature = 0;
    TimeStamp = std::time(0);

    /*The two PID implementations*/
    this->AbsPID = new PID(&CurrentTemperature, &TOutput, &SetTemperature, KpA, KiA, KdA, P_ON_M, DIRECT);
    this->RatePID = new PID(&TemperatureRateMovingAvg, &ROutput, &RSetpoint, KpR, KiR, KdR, P_ON_M, DIRECT);
    this->AbsPID->SetOutputLimits(MinPower, MaxPower);
    this->RatePID->SetOutputLimits(MinPower, MaxPower);
}

CryoControlSM::~CryoControlSM(void)
{
    delete this->AbsPID;
    delete this->RatePID;
}

void CryoControlSM::SMEngine(void)
{

    /*First, run the interaction with the SQL server with updates
     *and fresh changes*/

    DataPacket _thisDataSweep;
    this->UpdateVars(_thisDataSweep);

    /*Next look for changes*/

    /*Kp Ki Kd changes */
    if (_thisDataSweep.kpA != this->AbsPID->GetKp() || _thisDataSweep.kiA != this->AbsPID->GetKi() || _thisDataSweep.kdA != this->AbsPID->GetKd())
    {
        printf("\nTuning change!\n");
        this->AbsPID->SetTunings(_thisDataSweep.kpA, _thisDataSweep.kiA, _thisDataSweep.kdA);
    }
    if (_thisDataSweep.kpR != this->RatePID->GetKp() || _thisDataSweep.kiR != this->RatePID->GetKi() || _thisDataSweep.kdR != this->RatePID->GetKd())
    {
        printf("\nTuning change!\n");
        this->RatePID->SetTunings(_thisDataSweep.kpR, _thisDataSweep.kiR, _thisDataSweep.kdR);
    }

    /*Last sweep times*/
    this->LastHeaterArduinoTime = _thisDataSweep.LastHeaterArduinoTime;

    /*Temperature set point changes*/
    if (_thisDataSweep.targetTemp != this->SetTemperature)
    {
        printf("Temperature setpoint change!\n");
        this->SetTemperature = _thisDataSweep.targetTemp;
    }

    /*Now update the current and the last temperature. Also update the rate of change of temperature with the new information.*/
    this->LastTemperature = this->CurrentTemperature;

    /*REDO: Logic to decide if the LSH RTD is unplugged. An unplugged RTD will show a temperature
     *of ???*/

    if (_thisDataSweep.currentTempK2 < 350 && _thisDataSweep.currentTempK2 > 0)
    {
        this->CurrentTemperature = _thisDataSweep.currentTempK2;
        this->TemperatureMovingAvg = this->TemperatureMovingAvgK2;
    }
    else
    {
        this->CurrentTemperature = _thisDataSweep.currentTempK1;
        this->TemperatureMovingAvg = this->TemperatureMovingAvgK1;
    }

    //this->CurrentTemperature = _thisDataSweep.currentTemp;
    if (this->LastTemperature != 0)
        this->TemperatureRateMovingAvg += (this->CurrentTemperature - this->LastTemperature) / RateMovingAvgN - this->TemperatureRateMovingAvg / RateMovingAvgN;

    /*Store the two temperatures independently. Measurement > 0 is there to prevent values of 0 or so if the RTD is disconnected*/
    if (_thisDataSweep.currentTempK1 > 10 && _thisDataSweep.currentTempK1 < 500)
        this->TemperatureMovingAvgK1 += (_thisDataSweep.currentTempK1 - this->TemperatureMovingAvgK1) / RateMovingAvgN;
    if (_thisDataSweep.currentTempK2 > 10 && _thisDataSweep.currentTempK2 < 500)
        this->TemperatureMovingAvgK2 += (_thisDataSweep.currentTempK2 - this->TemperatureMovingAvgK2) / RateMovingAvgN;

    // LN2 Line Voltage
    this->CurrentLN2Valve = _thisDataSweep.CurrentLN2Valve;
    this->CurrentLN2ValveState = _thisDataSweep.CurrentLN2ValveState;
    this->RTDVoltage = _thisDataSweep.RTDVoltage;
    this->ValveSwitchTimestamp = _thisDataSweep.ValveSwitchTimestamp;
    for (int i = 0; i < 3: ++i){
    	this->PreviousRTDVoltages[i] = _thisDataSweep.PreviousRTDVoltages[i];
    }

    //cup resistance top
    if (_thisDataSweep.CupTempR0 > 0 && _thisDataSweep.CupTempR0<130){
        this->CupTempAvgTop += (_thisDataSweep.CupTempT0 - this->CupTempAvgTop) / RateMovingAvgN;
   	//this->CupRAvgTop += (_thisDataSweep.CupTempR0 - this->CupRAvgTop) / RateMovingAvgN;
    }
    this->CupControlTempAvgLast = this->CupTempAvgTop;

    // Compute the time in the current LN2 valve state
    time(&NowTime);
    this->TimeInCurrentLNState += difftime(NowTime, PreviousTime);
    PreviousTime = NowTime;

    /*Decide what state the system should be in. Then run the function to switch state if needed.*/
    this->StateDecision();
    if (this->CurrentFSMState != this->ShouldBeFSMState)
        this->StateSwitch();

    /*Finally, run the state function. Note: This probably should be run between decision and switch if one wants to use exit guards.*/
    (this->*CryoStateFn)();

    /*Perform post run operations and sanity checks*/
    this->PostRunSanityCheck();
}

void CryoControlSM::PostRunSanityCheck(void)
{

    /* Catastrophe prevention
     * Condition: Temperature is over 320 C.
     * Action: Both CC Power and Heater power set to 0.
     */

    this->ThisRunHeaterPower = this->ThisRunPIDValue;

    if (this->TemperatureMovingAvg > 320 || this->CurrentTemperature > 350)
    {
        this->ThisRunHeaterPower = 0.0;
        this->LN2Interlock = 1;
    }
}

void CryoControlSM::StateDecision(void)
{

    /*If the SM is turned off (manual mode), then the state should be idle and no output is produced.*/
    if (this->FSMMode == MANUAL)
    {
        this->ShouldBeFSMState = ST_Idle;
        return;
    }

    /*Warmup State conditions*/
    if (this->CurrentTemperature > 300 && this->SetTemperature < 300)
        this->ShouldBeFSMState = ST_Idle; //this should never happen in practice. If it does, then idle.

    if (this->SetTemperature > this->CurrentTemperature + 20 &&
        this->CurrentTemperature < 300)
        this->ShouldBeFSMState = ST_Warmup;

    /*Cooldown while the current temperature is high - currentTemp > setTemp*/
    if (this->SetTemperature < this->CurrentTemperature - 20){

        // Check if system has run out of Ln2
        if(this->ThisRunValveState == 1 && this->TimeInCurrentLNState > MaximumValveOpenTime*60){
            // Update target temperature
            this->UpdateTargetTemperature(300);
            this->ShouldBeFSMState = ST_Warmup;        }
        else{
            this->ShouldBeFSMState = ST_CoolDown;

        }
    }

    /*Maintain a cold state once the temperature is within 20 K of set point*/
    if (std::fabs(this->SetTemperature - this->CurrentTemperature) <= 20 && this->SetTemperature < 220){

        // Check if system has run out of Ln2
        if(this->ThisRunValveState == 1 && this->TimeInCurrentLNState > MaximumValveOpenTime*60){
            // Update target temperature
            this->UpdateTargetTemperature(300);
            this->ShouldBeFSMState = ST_Warmup;
        }
        else{
            this->ShouldBeFSMState = ST_MaintainCold;

        }
    }

    /*Maintain a warm state once the temperature is within 20 K of set point while warming up*/
    if (std::fabs(this->SetTemperature - this->CurrentTemperature) <= 20 && this->SetTemperature >= 220)
        this->ShouldBeFSMState = ST_MaintainWarm;


    /* If LN2 valve has been open for more than MaximumValveOpenTime, switch to warmup */
    /*
     * If the arduino is not responding
     */
    time(&NowTime);
    int LastDeltaArdHeater = difftime(NowTime, this->LastHeaterArduinoTime);

    if (LastDeltaArdHeater > 30 && LastDeltaArdHeater < 60)
        printf("There has been no communication from the Arduino heater for (%d) seconds!\n", LastDeltaArdHeater);
    if (LastDeltaArdHeater > 60)
    {
        printf("Fault: No communication from Arduino heater.\n");
        this->ShouldBeFSMState = ST_Fault;
    }
}

void CryoControlSM::StateSwitch(void)
{

    /*Activate entry guard*/
    this->EntryGuardActive = true;

    /*Switch the funtion pointer to the should be state function*/
    this->CryoStateFn = this->STFnTable[ShouldBeFSMState];

    /*Switch the state of the machine*/
    this->CurrentFSMState = this->ShouldBeFSMState;
}

void CryoControlSM::Warmup(void)
{

    /*Entry guard function: Activate rate PID. Set the rate target for RatePID.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive)
    {

        /*Activate the correct PID*/
        this->AbsPID->SetMode(MANUAL);
        this->RatePID->SetMode(AUTOMATIC);

        /*Set the correct rate direction for the rate*/
        this->RSetpoint = DeltaTRatePerMin / 60.0;

        /*Cold switch off*/
        this->ThisRunValveState = 0;
        this->LN2Interlock = 1;

        /*Turn SRS off*/
        this->ComputedSRSPowerState = 0;

        /*Guard done*/
        this->EntryGuardActive = false;
    }

    /*Calculate Rate PID*/
    this->RatePID->Compute();
    this->ThisRunPIDValue = this->ROutput;
}

void CryoControlSM::Idle(void)
{

    /*Entry guard function: Deactivate all PIDs.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive)
    {

        this->AbsPID->SetMode(MANUAL);
        this->RatePID->SetMode(MANUAL);

        /*Cold switch off*/
        this->LN2Interlock = 1;

        /*Turn SRS off*/
        this->ComputedSRSPowerState = 0;

        this->EntryGuardActive = false;
    }

    /*System is idle - so heater should be OFF*/
    this->ThisRunPIDValue = 0.0;
}

void CryoControlSM::Fault(void)
{

    /*Entry guard function: Deactivate all PIDs.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive)
    {

        this->AbsPID->SetMode(MANUAL);
        this->RatePID->SetMode(MANUAL);

        /*Cold switch off*/
        this->LN2Interlock = 1;

        /*Turn SRS off*/
        this->ComputedSRSPowerState = 0;

        this->EntryGuardActive = false;
    }

    /*System is at fault - so heater should be OFF*/
    this->ThisRunPIDValue = 0.0;
    this->ThisRunHeaterPower = 0.0;
}

void CryoControlSM::LN2FlowCheck(void)
{
	time(&NowTime);
	int TimeSinceValveSwitch = difftime(NowTime, this->ValveSwitchTimestamp);

	if (this->CurrentLN2ValveState == 1 && TimeSinceValveSwitch > 300) {
		if (this->RTDVoltage <= this->PreviousRTDVoltages[0] || this->RTDVoltage <= this->PreviousRTDVoltages[1] || this->RTDVoltage <= this->PreviousRTDVoltages[2]) {
			// LN2 is flowing
		} else {	// LN2 is not flowing
			this->CurrentLN2Valve = 11 - this->CurrentLN2Valve;
			this->ValveSwitchTimestamp = time(&NowTime);
		}
	}
}
/*Note: CooldownHot possibly requires
 *PID limits to be overriden
 *since 75% power seems to be too little
 *to get the rate at <5 / min
 */

void CryoControlSM::CoolDown(void)
{
    /*Entry guard function: Activate rate PID. Set the rate target for RatePID.
     *Turn on cryocooler.
     */
    if (this->EntryGuardActive)
    {

        /*Activate the correct PID*/
        this->AbsPID->SetMode(MANUAL);
        this->RatePID->SetMode(AUTOMATIC);

        /*Set the correct rate direction for the rate*/
        this->RSetpoint = -1.0 * DeltaTRatePerMin / 60.0; // degrees per sec

        /*LN interlock off*/
        this->LN2Interlock = 0;

        /*Turn SRS off*/
        this->ComputedSRSPowerState = 0;

        this->EntryGuardActive = false;
    }

    /*Calculate Rate PID*/
    this->RatePID->Compute();
    this->ThisRunPIDValue = this->ROutput;
    this->LN2FlowCheck();

    // Perform LN2 valve determination
    if(ThisRunValveState == 0 && (this->TimeInCurrentLNState > TimeBetweenFillCooldown*60 || this->CurrentTemperature > 270)){
            ThisRunValveState = 1;
            this->TimeInCurrentLNState = 0;
    }

    if(!IsOverflow && this->CupTempAvgTop < TopRTDFilledThreshold && ThisRunValveState == 1 && this->TimeInCurrentLNState > MinimumTimeBeforeOverflow*60){
        IsOverflow = 1;
        this->TimeInCurrentLNState = 0;
    }

    if(IsOverflow && this->TimeInCurrentLNState > TimeAfterOverflow*60 && ThisRunValveState == 1 ){
        ThisRunValveState = 0;
        IsOverflow = 0;
        this->TimeInCurrentLNState = 0;
    }
}

void CryoControlSM::MaintainWarm(void)
{

    /*Entry guard function: Activate AbsPID.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive)
    {

        /*Activate the correct PID*/
        this->AbsPID->SetMode(AUTOMATIC);
        this->RatePID->SetMode(MANUAL);

        /*Cold switch off*/
        this->ThisRunValveState = 0;
        this->LN2Interlock = 1;

        /*Turn SRS ON if */
        this->ComputedSRSPowerState = 0;

        this->EntryGuardActive = false;
    }

    /*Calculate PID*/
    this->AbsPID->Compute();
    this->ThisRunPIDValue = this->TOutput;
}

void CryoControlSM::MaintainCold(void)
{

    /*Entry guard function: Activate AbsPID.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive)
    {

        /*Activate the correct PID*/
        this->AbsPID->SetMode(AUTOMATIC);
        this->RatePID->SetMode(MANUAL);

        /*Cold switch off*/
        this->LN2Interlock = 0;

                /*Turn SRS ON if */
        this->ComputedSRSPowerState = 1;

        this->EntryGuardActive = false;
    }

    /*Calculate PID*/
    this->AbsPID->Compute();
    this->ThisRunPIDValue = this->TOutput;
    this->LN2FlowCheck();

    // LN2 Valve decision
    if(ThisRunValveState == 0 && this->TimeInCurrentLNState > TimeBetweenFillMaintainCold*60){
        ThisRunValveState = 1;
        this->TimeInCurrentLNState = 0;
    }

    //if(!IsOverflow && OverflowVoltage > LN2OverflowVoltageThreshold && ThisRunValveState == 1){
    //    IsOverflow = 1;
    //    TimeInCurrentLNState = 0;
    //}

    if(!IsOverflow && this->CupTempAvgTop < TopRTDFilledThreshold && ThisRunValveState == 1 && this->TimeInCurrentLNState > MinimumTimeBeforeOverflow*60){
        IsOverflow = 1;
        this->TimeInCurrentLNState = 0;
    }


    if(IsOverflow && this->TimeInCurrentLNState > TimeAfterOverflow*60 && ThisRunValveState == 1 ){
        ThisRunValveState = 0;
        IsOverflow = 0;
        this->TimeInCurrentLNState = 0;
    }
}

/*The functions to access a copy of variables for viewing*/
double CryoControlSM::getCurrentTemperature(void) { return this->CurrentTemperature; }
double CryoControlSM::getTargetTemperature(void) { return this->SetTemperature; }
double CryoControlSM::getCurrentPIDValue(void) { return this->ThisRunPIDValue; }
double CryoControlSM::getTemperatureRate(void) { return this->TemperatureRateMovingAvg; }
double CryoControlSM::getTemperature(void) { return this->TemperatureMovingAvg; }
double CryoControlSM::getTemperatureSP(void) { return this->SetTemperature; }
double CryoControlSM::getTRateSP(void) { return this->RSetpoint; }
int CryoControlSM::getCurrentState(void) { return (int)this->CurrentFSMState; }
int CryoControlSM::getShouldBeState(void) { return (int)this->ShouldBeFSMState; }
