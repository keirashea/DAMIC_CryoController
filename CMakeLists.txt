cmake_minimum_required(VERSION 2.8)
project( DAMICMCryoCtrl )
set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_FLAGS "-Wno-write-strings -std=gnu++1y -I/usr/include/mysql-cppconn-8 -march=native -m64")
set (CMAKE_EXE_LINKER_FLAGS "-Wno-write-strings -lmysqlcppconn8 -lm")


set(SOURCEARD
    ${CMAKE_CURRENT_SOURCE_DIR}/ArduinoHeater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.cpp
)

set(HEADERARD
    ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ArduinoHeater.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MysqlCredentials.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.hpp
)


set(SOURCEARDTEMP
    ${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCupTemp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.cpp
)

set(HEADERARDTEMP
    ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCupTemp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MysqlCredentials.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.hpp
)



set(SOURCEPFEIF
        ${CMAKE_CURRENT_SOURCE_DIR}/PfeifferVacuumGauge.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.cpp
)

set(HEADERPFEIF
        ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PfeifferVacuumGauge.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MysqlCredentials.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.hpp
)

set(SOURCELN2
        ${CMAKE_CURRENT_SOURCE_DIR}/LN2Controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.cpp
        )
set(HEADERLN2
        ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LN2Controller.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MysqlCredentials.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.hpp
        )


set(SOURCESM
    ${CMAKE_CURRENT_SOURCE_DIR}/CryoControlSM.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PID_v1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CCMySQLInteraction.cpp
)
set(HEADERSSM
    ${CMAKE_CURRENT_SOURCE_DIR}/PID_v1.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CryoControlSM.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MysqlCredentials.hpp
)



set(SOURCESRS
   ${CMAKE_CURRENT_SOURCE_DIR}/SRSPowerSupply.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.cpp
)
set(HEADERSSRS
   ${CMAKE_CURRENT_SOURCE_DIR}/SRSPowerSupply.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/MysqlCredentials.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/SerialDeviceT.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/UtilityFunctions.hpp
)


add_executable( ArdHeaterDrone ArduinoHeaterDroneStart.cpp MysqlCredentials.hpp)
target_link_libraries( ArdHeaterDrone ${SOURCEARD} ${HEADERARD})

add_executable( PfeifferDroneStart PfeifferDroneStart.cpp MysqlCredentials.hpp)
target_link_libraries( PfeifferDroneStart ${SOURCEPFEIF} ${HEADERPFEIF})

add_executable( LN2DroneStart LN2DroneStart.cpp MysqlCredentials.hpp)
target_link_libraries( LN2DroneStart ${SOURCELN2} ${HEADERLN2})

add_executable( StartSM StartSM.cpp MysqlCredentials.hpp)
target_link_libraries( StartSM ${SOURCESM} ${HEADERSSM})


add_executable( ArdTemp ArduinoCupTempDrone.cpp MysqlCredentials.hpp)
target_link_libraries( ArdTemp ${SOURCEARDTEMP} ${HEADERARDTEMP})


# ------------------------------- #

find_library(rpclib rpc ${CMAKE_CURRENT_SOURCE_DIR}/rpclib/ REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rpclib/)

add_executable(SRSSrv SRSServer.cpp)
target_link_libraries(SRSSrv ${rpclib} ${SOURCESRS} ${HEADERSSRS} pthread)
set_property(TARGET SRSSrv PROPERTY CXX_STANDARD 14)



