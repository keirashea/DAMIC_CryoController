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
#include <vector>

class ArdCupTemp : public SerialDevice {
public:
    ArdCupTemp(std::string );
    ~ArdCupTemp();
    void ReadResistor();
    void CalculateTemp(void );
    void UpdateMysql(void);

    std::vector<int> RTDResistance;

    bool WatchdogFuse;
    std::string SQLStatusMsg;



    int _cHeaterMode, _cWatchdogFuse;


};

#endif
