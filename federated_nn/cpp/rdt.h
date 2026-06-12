#pragma once
#include "protocol.h"
#include "udp_socket.h"
#include <vector>
#include <string>
#include <functional>

namespace rdt {

using LogFn = std::function<void(const std::string&)>;

class RDTNode {
public:
    RDTNode(int listen_port, uint16_t my_node_id = 0, LogFn log_fn = nullptr);


    int send(const std::string& dest_ip, int dest_port,
             uint16_t dest_id, uint8_t type,
             const std::vector<uint8_t>& data);


    struct Message {
        uint8_t              type = 0;
        std::vector<uint8_t> data;
        bool empty() const { return data.empty(); }
    };
    Message recv(int timeout_ms = 15000);

private:
    void send_one(const Packet& pkt, const std::string& ip, int port);
    void send_response(uint8_t resp_type, uint32_t seq,
                       const std::string& ip, int port);


    void print_packet(const Packet& pkt, const std::string& direction) const;

    void log(const std::string& msg) const { if (log_fn_) log_fn_(msg); }

    UDPSocket sock_;
    uint16_t  my_id_;
    LogFn     log_fn_;
};

} 
