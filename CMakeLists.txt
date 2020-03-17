cmake_minimum_required(VERSION 2.8)
project( DAMICMCryoCtrl )
set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_FLAGS "-Wno-write-strings -std=gnu++1y -I/usr/include/mysql-cppconn-8 -O0 -march=native -m64")
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

add_executable( ArdHeaterDrone ArduinoHeaterDroneStart.cpp MysqlCredentials.hpp)
target_link_libraries( ArdHeaterDrone ${SOURCEARD} ${HEADERARD})

add_executable( PfeifferDroneStart PfeifferDroneStart.cpp MysqlCredentials.hpp)
target_link_libraries( PfeifferDroneStart ${SOURCEPFEIF} ${HEADERPFEIF})

add_executable( LN2DroneStart LN2DroneStart.cpp MysqlCredentials.hpp)
target_link_libraries( LN2DroneStart ${SOURCELN2} ${HEADERLN2})

add_executable( StartSM StartSM.cpp MysqlCredentials.hpp)
target_link_libraries( StartSM ${SOURCESM} ${HEADERSSM})

