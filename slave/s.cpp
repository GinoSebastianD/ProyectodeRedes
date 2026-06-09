// g++ server.cpp -o server -pthread



#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

constexpr int PORT = 9000;

int payloadsize = 493;

map<string, sockaddr_in> clients;
map<string, vector<string>> fragmentBuffer;


mutex clientsMutex;

string fill(int num, int digits)
{
    string s = to_string(num);
    int diff = digits - (int)s.size();
    if (diff > 0) s.insert(0, diff, '0');
    return s;
}

void paddling(string &datagram,int size){
    int diff = size - datagram.size();
    
    //cout<< size <<" - "<< datagram.size()<< " = "<< diff<<endl;
    if (diff > 0) datagram.insert(datagram.size(),diff,'#');
    //cout<< size <<" - "<< datagram.size()<< " = "<< diff<<endl;
    
    return;
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

void send_data(int sockfd, sockaddr_in serverAddr, vector<string>data){
  
    string datagram;
    for(int x{0};x<data.size();x++){
        string datagram;
        
        //HEADER
        if(x==data.size()-1){
            datagram += "11";
        }
        else if(x == 0){
            datagram += "01";  
        }
        else{
            datagram += "00";
        }
        
        datagram += fill(x,4);
        
        //payload
        
        datagram += data[x];
        
        // paddling
        
        paddling(datagram,499);
        
        //checksum
        int checksum = 0;
        for(auto k : datagram){
            checksum+= k;
        }
        datagram+= fill(checksum %7,1);
        
        cout<<datagram<<endl;
        sendto(sockfd, datagram.c_str(), 500, 0, (sockaddr *)&serverAddr, sizeof(serverAddr));
      
    }
    cout << "Data sent successfully.\n";
    
  
}

string buildProtocol(
    char action,
    const string &nickname,
    const string &destination,
    const string &message,
    const string &filename,
    const string &fileData
) {

    string protocol;

    // ACTION
    protocol += action;

    // NICKNAME
    protocol +=
        fill(nickname.size(), 3);

    protocol += nickname;

    // DESTINATION
    protocol +=
        fill(destination.size(), 3);

    protocol += destination;

    // MESSAGE
    protocol +=
        fill(message.size(), 5);

    protocol += message;

    // FILENAME
    protocol +=
        fill(filename.size(), 11);

    protocol += filename;

    // FILE DATA
    protocol +=
        fill(fileData.size(), 20);

    protocol += fileData;

    return protocol;
}


string buildClientKey(sockaddr_in addr) {

    return string(inet_ntoa(addr.sin_addr))
           + ":"
           + to_string(ntohs(addr.sin_port));
}

void receiveThread(
    int sockfd,
    string fullData,
    sockaddr_in clientAddr
) {

    //processMessage(sockfd, fullData, clientAddr);
}

int main() {

    int sockfd =
        socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(
        sockfd,
        (sockaddr *)&serverAddr,
        sizeof(serverAddr)
    );

    cout << "UDP CHAT SERVER RUNNING\n";

    
    while (true){
      
      char buffer[500];

      sockaddr_in clientAddr{};
      socklen_t len = sizeof(clientAddr);
      
      int received = recvfrom(sockfd,buffer,500,0,(sockaddr *)&clientAddr,&len);
      
      int current{0};
      
      string flag(buffer + current, 2);
      current +=2;
      
      string seq(buffer + current, 4);
      current +=4;
      
      
      string clientKey = buildClientKey(clientAddr);
        
      string datagram(buffer, 500);

      fragmentBuffer[clientKey].push_back(datagram);
      
      
      if(flag == "11"){
        
        cout<<"---11--"<<endl;
        string fragmento = fragmentBuffer[clientKey][0];
        
        char type = fragmento[current];
        current++;
        
        string parte = fragmento.substr(current, 3);
        //cout<<"PARTE STOI: "<<parte<<endl;
        int nickorsize = stoi(parte);
        current += 3;

        string nickor = fragmento.substr(current, nickorsize);
        current += nickorsize;

        parte = fragmento.substr(current, 3);
        int nickdestsize = stoi(parte);
        current += 3;

        string nickdest = fragmento.substr(current, nickdestsize);
        current += nickdestsize;
        
        cout<<type<<endl;
        
        if(type == 'L'){
          clients[nickor] = clientAddr;
          for (auto &k : clients) {
            cout<< k.first<<endl;
          }
          fragmentBuffer[clientKey].pop_back();
          continue;
        }
        
        if(type == 'O'){
          clients.erase(nickor);
          cout<< nickor<<" eliminado"<<endl;
          fragmentBuffer[clientKey].pop_back();
          continue;
        }
        
        if(type == 'T'){
          
          string lista;

          for (auto &k : clients){
              lista += k.first + "\n";
          }
          
          for (const auto& [key, addr] : clients) {
              cout << "Cliente: " << key << endl;

              cout << "IP: "
                   << inet_ntoa(addr.sin_addr)
                   << endl;

              cout << "Puerto: "
                   << ntohs(addr.sin_port)
                   << endl;
          }

          // construir respuesta
          string protocol;

          protocol += 'T'; // acción de respuesta

          protocol += fill(lista.size(),10);
          
          protocol += lista;

          vector<string> storage = split(protocol);
          
          fragmentBuffer[clientKey].pop_back();
          send_data(sockfd, clientAddr, storage);

          continue;
        }
        
        if(type == 'B'){
          for(int i = 0; i < fragmentBuffer[clientKey].size(); i++){
            fragmento = fragmentBuffer[clientKey][i];
            for (auto &k : clients) {  
              sendto(sockfd, fragmento.c_str(), 500, 0, (sockaddr *)&k.second, sizeof(k.second));
            }
          fragmentBuffer[clientKey].pop_back();
          }
          continue;
        }
          
        
        sockaddr_in addr = clients[nickdest];
        
        for(int i = 0; i < fragmentBuffer[clientKey].size(); i++){
            fragmento = fragmentBuffer[clientKey][i];
            sendto(sockfd, fragmento.c_str(), 500, 0, (sockaddr *)&addr, sizeof(addr));
        }
        fragmentBuffer[clientKey].clear();
        
      }
      
      cout<<"ver: "<<datagram<<endl;
      
    }
  
  

    close(sockfd);

    return 0;
}