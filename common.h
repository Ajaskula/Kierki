#ifndef COMMON_H
#define COMMON_H

#include<string>
#include<vector>
#include<iostream>
#include<ctime>
#include<chrono>
#include<sstream>
#include<iomanip>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> 
#include "cards.h"
#include <regex>
#include <set>
#include <fcntl.h>
#include <cstdarg>
#include <string.h>

#define MESSAGE_LIMIT 1000

// Function declarations

// Function that prints error message and exits the program
[[noreturn]] void fatal(const char* fmt, ...);
// Function that prints error message with errno and exits the program
[[noreturn]] void syserr(const char* fmt, ...);
// Function that reads port from string
uint16_t read_port(char const *string);
// Function that returns current time in string format
std::string get_current_time();
// Function that returns server address in string format
std::string get_server_address(int socket_fd);
// Function that returns local address in string format
std::string get_local_address(int socket_fd);
// Function that prints report
void raport(const std::string& addr1, const std::string& addr2, const std::string& message);
// Function that returns string with card list
std::vector<std::string> extract_hand(const std::string& hand);
// Function that checks if string is correct card list
bool is_string_correct_card_list(const std::string& hand);
// Function that checks if string is correct card
bool is_valid_request_to_send_card(const std::string& input);
// Function that returns card from string
std::string get_busy_places_from_BUSY(const std::string& message);
// Function that returns card from string
std::string convert_total_message(const std::string& message);
// Function that returns card from string
std::string extract_card_list_from_taken(const std::string& message);
// Function that returns card from string
std::string convert_score_message(const std::string& message);
// Function that returns card from string
std::vector<std::string> extract_card_vector_from_taken(const std::string& message);
// Function that returns card from string
char get_first_card_color_from_TRICK(const std::string& message);
// Function that returns card from string
std::string convert_taken_message(const std::string& taken_message);
// Function that returns card from string
std::string format_card_list(const std::string& card_list); 
// Function that returns card from string
std::string get_list_of_cards_from_TRICK(const std::string& trick, int current_trick);

#endif // COMMON_H