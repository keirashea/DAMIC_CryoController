/*
 * UDPClient.cpp
 *
 *  Created on: Mar 21, 2022
 *      Author: keirahansen
 */


// UDP Client Server -- send/receive UDP packets
// UDP client classes to send datagram to arduino server


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "UDPClient.hpp"

#define MAX_LEN 64
#define PORT_NUM 8888
//cenpa_mac[] = {AC, 1F, 6B, 7B, 83, 89}

UDPClient::UDPClient(std::string server_ip)
{

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (socket_fd==-1) {
	    return ;
	}
	// Set port and IP:
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	//server_addr.sin_addr = inet_addr((const char *)server_ip);


	// Bind to the set port and IP:
	if(bind(socket_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		return ;
	}
	return ;
}

UDPClient::~UDPClient()
{
	close(socket_fd);
}


char* UDPClient::ExchangePackets(char* data_buffer)
{
	// sends buffer to server
	//char data_buffer[MAX_LEN];
	memset(data_buffer, '\0', sizeof(data_buffer));

	if (sendto(socket_fd, data_buffer, sizeof(data_buffer), 0, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		return -1;
	}

	// receive buffer from server
	char receiving_buffer[MAX_LEN];
	memset(receiving_buffer, '\0', sizeof(receiving_buffer));
	socklen_t addr_len;

	if(recvfrom(socket_fd, receiving_buffer, MAX_LEN, 0, (struct sockaddr*) &server_addr, &addr_len) < 0){
		return -1;
	} else {
		return receiving_buffer;
	}


}

