#include "calculator.hpp"
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

using namespace std;
constexpr int PORT = 9000;
int payloadsize = 490;

int Mactual=1;

namespace calculator {
    double add(double a, double b) {
        return a + b;
    }

    double subtract(double a, double b) {
        return a - b;
    }

    double multiply(double a, double b) {
        return a * b;
    }

    double divide(double a, double b) {
        if (b == 0) {
            throw "Division by zero error";
        }
        return a / b;
    }
    
    ///////////////////////////////NUEVO/////////////////////////////////////////
  
  
  
  
    string fill(int num, int digits)
    {
        string s = to_string(num);
        int diff = digits - (int)s.size();
        if (diff > 0) s.insert(0, diff, '0');
        return s;
    }
    
    string paddling(std::string datagram, int size)
    {
        int diff = size - datagram.size();

        if (diff > 0)
            datagram.insert(datagram.size(), diff, '#');

        return datagram;
    }
    
    UDPClient::UDPClient(const std::string& ip,int port) // constructor del cliente
    {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        inet_pton(
            AF_INET,
            ip.c_str(),
            &serverAddr.sin_addr
        );
      
        sockaddr_in clientAddr{};

        clientAddr.sin_family = AF_INET;
        clientAddr.sin_addr.s_addr = INADDR_ANY;
        clientAddr.sin_port = htons(port);

        if(bind( sockfd, (sockaddr*)&clientAddr, sizeof(clientAddr)) < 0)
        {
            std::cerr << "Error en bind" << std::endl;
        }
    }  
  
    UDPClient::~UDPClient()
    {
        close(sockfd);
    }
    
    void UDPClient::send_data(          // funciona que manda datagramas
        std::vector<std::string> data,
        int dest
    )
    {
        for(int x{0}; x<data.size(); x++)
        {
            string datagram;

            datagram += fill(dest,2);

            if(x == data.size()-1)
                datagram += "11";
            else if(x == 0)
                datagram += "01";
            else
                datagram += "00";

            datagram += fill(x,4);

            datagram += data[x];

            datagram = paddling(datagram,498);

            int checksum = 0;

            for(auto k : datagram)
                checksum += k;

            datagram.insert(
                0,
                fill(checksum % 100,2)
            );
            
            cout<<datagram<<endl;  
          
            sendto(
                sockfd,
                datagram.c_str(),
                500,
                0,
                (sockaddr*)&serverAddr,
                sizeof(serverAddr)
            );
            
        }
        cout << "Data sent successfully." << endl;
    }  
    
    string UDPClient::receive_message() {
        string data;
        char buffer[500];

        sockaddr_in senderAddr{};
        socklen_t len = sizeof(senderAddr);

        do {
            int received = recvfrom(sockfd, buffer, 500, 0, (sockaddr *)&senderAddr, &len);
            if (received <= 0) continue;
          
            int checksuml = 0;
              
            int checksumr = stoi(string(buffer, 2)); // 2 primeros bytes del datagrama
            
            for(int x{2};x<500;x++){
              checksuml+=buffer[x];
            }
            checksuml = checksuml % 100;  
          
            if(checksuml != checksumr){
              cout<<"incorrecto"<<endl;
              continue;
            }
            
            string nodeid(buffer+2, 2);
          
            string flag(buffer+4, 2);
          
            string seq(buffer+6, 4);   


            string payload(buffer + 10, 490);
            data += payload;


            if (flag == "11") {
                break;
            }

        } while (true);

        return data;
    }
  
    
    void UDPClient::parseProtocol(const string &data) {

        int idx = 0;
      
        char action = data[idx];
        
        idx ++;
      
        int datasize = stoi(data.substr(idx, 10));

        idx +=10;
      
        string fileData = data.substr(idx, datasize);
        idx+=datasize;
      
        if(action == 'M'){
          ofstream file("pesos"+ to_string(Mactual)+ ".txt" );

          if(file.is_open())
          {
              file << fileData;
              file.close();

              std::cout << "Pesos creado correctamente" << std::endl;
          }
          
          if(Mactual == 3){ // para que sea pesos1.txt o pesos2.txt o pesos3.txt
            Mactual=1;
          }
          else{
            Mactual++;
          }
          
        }
        if(action == 'D'){
          
          ofstream file("Dataset of Diabetes.csv");

          if(file.is_open())
          {
              file << fileData;
              file.close();

              std::cout << "Data creado correctamente" << std::endl;
          }
        }
    }
  
    void UDPClient::fread(){
  
        string data = receive_message();
        parseProtocol(data);

    }
    
  
    vector<string> split(string data){
        int totalfrags = (data.size() + payloadsize - 1) / payloadsize;
        int left= data.size();
        int current=0 ;
        vector<string> chunks;
        for (int x{0}; x<totalfrags ;x++){
            string aux;
            if(left < payloadsize){
                aux = data.substr(current,left);
                current+= left;
                left -= left;
            }
            else{
                aux = data.substr(current,payloadsize);
                current += payloadsize;
                left -= payloadsize;
            }
            chunks.push_back(aux);
        }

        return chunks;
    }
  
  
    string buildProtocol( char action, const string &fileData){ // construye todo el protocolo 

        string protocol;

        // ACTION
        protocol += action;

        // FILE DATA
        protocol +=
            fill(fileData.size(), 10);

        protocol += fileData;

        return protocol;
    }
  
  
} 