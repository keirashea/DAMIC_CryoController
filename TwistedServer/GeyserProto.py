import netstring2, socket
import struct, random

host = 'localhost'
port = 55555
size = 10
CRLF = "\r\n"


def geyserCMD(command, expectreply = True):
    cmd_netstringed = netstring2.dumps(str(command))
    #print cmd_netstringed
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host,port))
    s.send(cmd_netstringed)
    response = None
    if expectreply:
        data = s.recv(15)
        response = netstring2.loads(data)
    else:
        response = "OK"
    s.close()
    return response
    

def getPressure():
    return float(geyserCMD("CurrentPressure.0"))

def getTC():
    return geyserCMD("CTC")

def getPW():
    return geyserCMD("HPW")


if __name__ == '__main__':
    print getPW()
    #print getTemperature()
