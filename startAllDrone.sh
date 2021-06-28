#!/bin/bash

# Kill all screens if they exist
screen -XS StateMachine quit
screen -XS ArduinoHeater quit
screen -XS LN2Controller quit
screen -XS PfeifferDrone quit
screen -XS SRSServer quit

# Start all screen sessions
screen -S SRSServer "./SRSSrv"
screen -S PfiefferDrone "./PfeifferDroneStart"
screen -S LN2Controller "./LN2DroneStart"
screen -S ArduinoHeater "./ArdHeaterDrone"
screen -S StateMachine "./StartSM"