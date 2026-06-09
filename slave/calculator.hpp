#pragma once

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string>

namespace calculator {
    
    
    
    double add(double a, double b);
    double subtract(double a, double b);
    double multiply(double a, double b);
    double divide(double a, double b);
    
    ///////////////// nuevo ///////////////////////////////
    
    std::string fill(int num, int digits);

    std::string paddling(std::string datagram, int size);
    
    class UDPClient
    {
    public:
        int sockfd;
        sockaddr_in serverAddr;

        UDPClient(const std::string& ip, int port);
        ~UDPClient();

        void send_data(std::vector<std::string> data, int dest);
    };
    
    std::vector<std::string> split(std::string data);

    std::string buildProtocol(char action, const std::string& fileData);
    
} 