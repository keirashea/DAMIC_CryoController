/*
 * UDPClient.hpp
 *
 *  Created on: Mar 21, 2022
 *      Author: keirahansen
 */
//#include <iostream>
#include <stdlib.h>
#include <string>
#include <netinet/in.h>

#ifndef UDPCLIENT_HPP_
#define UDPCLIENT_HPP_


class UDPClient
{
public:
    UDPClient(std::string );
    ~UDPClient();
    char* ExchangePackets(char*);

    int socket_fd;
	struct sockaddr_in server_addr;
};


#endif /* UDPCLIENT_HPP_ */
