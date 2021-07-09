//
//  CryoControlSM.hpp
//  CC_R
//
//  Created by Pitam Mitra on 1/6/19.
//  Copyright © 2019 Pitam Mitra. All rights reserved.
//

#ifndef CryoControlSM_hpp
#define CryoControlSM_hpp

#include <stdio.h>
#include "PID_v1.h"
#include <map>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iomanip>
#include <ctime>

#define RateMovingAvgN 20
#define DeltaTRatePerMin 1.0
#define TimeBetweenFillCooldown 20.0 // minutes
#define TimeBetweenFillMaintainCold 90.0 // minutes
#define TimeAfterOverflow 0.5// minute
#define LN2OverflowVoltageThreshold 2.55
#define TopRTDFilledThreshold 101
#define MinimumTimeBeforeOverflow 8.0 // minutes

struct DataPacket
{

    double kpA;
    double kiA;
    double kdA;

    double kpR;
    double kiR;
    double kdR;

    double targetTemp;
    double currentTempK1;
    double currentTempK2;

    double PID;
    double SystemState;

    bool WatchdogFuse;

    double overflowVoltage;

    time_t LastHeaterArduinoTime;
    time_t LastLNArduinoTime;

    bool SRSPowerState;
    bool LN2ValveInterLock;

    //cup RTDs
    float CupTempR0;
    float CupTempR1;
    float CupTempT0;
    float CupTempT1;
};

class CryoControlSM
{

private:
    double LastTemperature;
    unsigned long TimeStamp;

    double TInput, TOutput;
    double RInput, ROutput;

    /*SM variables and memory*/
    double KpA = 2, KiA = 5, KdA = 1;
    double KpR = 2, KiR = 5, KdR = 1;

    /*Process related variables*/
    double ThisRunPIDValue = 0.0;
    double ThisRunHeaterPower = 0.0;
    double CurrentTemperature = 0.0;
    double SetTemperature = 0.0;
    double TemperatureRateMovingAvg = 0.0;
    double TemperatureMovingAvgK1 = 0.0;
    double TemperatureMovingAvgK2 = 0.0;
    double RSetpoint = 0.0;

    // LN2 Variables
    int ThisRunValveState = 0;
    double OverflowVoltage = 0;
    bool LN2Interlock = 0;
    long TimeInCurrentLNState = 0;
    bool IsOverflow = 0;

    //cup temp
    double CupTempAvgTop = 300;
    double CupTempAvgMid = 300;
    double CupRAvgTop = 0;
    double CupRAvgMid = 0;
    double CupControlTempAvgLast;

    double TemperatureMovingAvg = 0.0;


    time_t LastHeaterArduinoTime;
    time_t LastLNArduinoTime;
    time_t PreviousTime = 0;
    time_t NowTime;

    bool ComputedSRSPowerState = 0;

    /* ***********************************************************
     *We have two different PIDs.
     *
     *1. For controlling the absolute value of the temperature
     *   and is used around setpoints when stability is desired.
     *
     *2. For controlling the rate of ascent or descent of temperature
     * ***********************************************************/

    PID *AbsPID;
    PID *RatePID;

    /*SM Functions and states*/
    void UpdateVars(DataPacket &);

    void Idle(void);
    void CoolDown(void);
    void Warmup(void);
    void MaintainWarm(void);
    void MaintainCold(void);
    void Fault(void);

    /*Enum values of all the states that the FSM can be in*/
    enum FSMStates
    {
        ST_Idle,
        ST_CoolDown,
        ST_Warmup,
        ST_MaintainCold,
        ST_MaintainWarm,
        ST_Fault
    };

    /*Jump table function for the FSM states and the function pointer to the current state*/
    std::map<FSMStates, void (CryoControlSM::*)(void)> STFnTable;
    void (CryoControlSM::*CryoStateFn)(void);

    FSMStates CurrentFSMState;
    FSMStates ShouldBeFSMState;

    void StateDecision(void);
    void StateSwitch(void);

    bool EntryGuardActive = false;
    bool ExitGuardActive = false;
    bool FSMMode = AUTOMATIC;
    bool hotToCold = false;

public:
    CryoControlSM();
    ~CryoControlSM(void);

    /*The SM Engine to be run at every time interval*/
    void SMEngine(void);

    /*Functions to access a copy of variables for viewing*/
    double getCurrentTemperature(void);
    double getTargetTemperature(void);
    double getCurrentPIDValue(void);
    double getTemperature(void);
    double getTemperatureRate(void);
    double getTemperatureSP(void);
    double getTRateSP(void);
    int getTimeInCurrentLNState(void) {return this->TimeInCurrentLNState;};
    int getCurrentState(void);
    int getShouldBeState(void);
    double getSentCCPower(void);
    double getCupTempTop(void) {return this->CupTempAvgTop;};

    void PostRunSanityCheck(void);
};

#endif /* CryoControlSM_hpp */
