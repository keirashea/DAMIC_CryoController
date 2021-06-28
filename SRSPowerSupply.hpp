/* **************************************************************************************
 * Code containing the Routines required to communicate and operate the SRS Power Supply
 * Module.
 *
 * by Pitam Mitra 2020 for DAMIC-M
 * **************************************************************************************/


#ifndef SRS_HPP_INCLUDED
#define SRS_HPP_INCLUDED


/*Includes*/
#include <iostream>
#include "SerialDeviceT.hpp"

class SRSPowerSupply : public SerialDevice {
public:
    SRSPowerSupply(std::string );
    ~SRSPowerSupply();

    /*SRS Related Routines*/

    //Get Parameters
    template <typename T>
    T GetParameterFromSRS(std::string );
    //Set Parameters

    // Get PS Values
	float ReadPSVoltage();
	bool ReadPSOutput();
    std::string IDN();
    bool IsOVLD(void);

	// Set PS Values
	void WritePSVoltage(float voltage);
	void WritePSOutput(bool output);

    //Ramp gen
	void VoltageRamp(float startScanVoltage, float stopScanVoltage, float scanTime, bool display);

	//Updates
	void PerformSweep(void);
	void UpdateMysql(void);

    //Read memory
    float ReadMemPSVoltage() const {return currentVoltage; }



private:

	float currentVoltage;
	bool currentOutputStatus;
    bool OVLDStatus;


    bool WatchdogFuse;
    bool SRSPowerState;
    std::string SQLStatusMsg;



    int _cHeaterMode, _cWatchdogFuse;


};




#endif
