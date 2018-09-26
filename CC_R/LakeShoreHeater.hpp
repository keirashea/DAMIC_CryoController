/* **************************************************************************************
 * Code containing the CCD class that will have the entire definitions we will need
 * such as the erase procedure, the set biases and the CCD tuning etc
 *
 * by Pitam Mitra 2018 for DAMIC-M
 * **************************************************************************************/


#ifndef Lakeshore_HPP_INCLUDED
#define Lakeshore_HPP_INCLUDED


/*Includes*/
#include <iostream>
#include "SerialDeviceT.hpp"

class LakeShore : public SerialDevice {
public:
    LakeShore(std::string );
    ~LakeShore();
    void ReadPower();
    void SetPowerLevel(float PW);


    float currentPW;
    float setPW;

};

#endif
