#pragma once
#include <string>
#include <stdexcept>
#include <cstdint>

namespace rdt {

// Dirección IP 
struct Addr {
    std::string ip;
    int         port = 0;
};


class UDPSocket {
public:
    // bind_port
    explicit UDPSocket(int bind_port);
    ~UDPSocket();

    // No copiable
    UDPSocket(const UDPSocket&)            = delete;
    UDPSocket& operator=(const UDPSocket&) = delete;

   
    void send_to(const void* buf, int len,
                 const std::string& dest_ip, int dest_port);


    int recv_from(void* buf, int max_len,
                  int timeout_ms, Addr& src);

private:
    int fd_;
};

} 
