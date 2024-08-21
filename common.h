#ifndef COMMON_H
#define COMMON_H

#include<string>
#include<vector>
#include<iostream>
#include<ctime>
#include<chrono>
#include<sstream>
#include<iomanip>
#include <arpa/inet.h>    // Dla inet_ntop
#include <netinet/in.h>   // Dla sockaddr_in
#include <sys/socket.h>   // Dla socket, connect
#include <unistd.h> 
#include "cards.h"

std::string getCurrentTime();
std::string getServerAddress(int socket_fd);
std::string getLocalAddress(int socket_fd);
void raport(const std::string& addr1, const std::string& addr2, const std::string& message);
std::vector<std::string> extract_hand(const std::string& hand);



#endif // COMMON_H