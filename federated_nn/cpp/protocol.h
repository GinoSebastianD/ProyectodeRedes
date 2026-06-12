#pragma once
#include <cstdint>
#include <cstring>

namespace rdt {



//Tamaños 
constexpr int PACKET_SIZE  = 500;
constexpr int HEADER_SIZE  = 15;                      
constexpr int MAX_DATA     = PACKET_SIZE - HEADER_SIZE; 

//Flags 
constexpr uint16_t FLAG_START = 0x0001;
constexpr uint16_t FLAG_BODY  = 0x0000;
constexpr uint16_t FLAG_END   = 0x0003;   

//Tipos de mensaje 
//Maestro - Esclavo:
constexpr uint8_t TYPE_MATRIX       = 'M';  
constexpr uint8_t TYPE_DATA         = 'D';  
//Esclavo - Maestro / control:
constexpr uint8_t TYPE_MATRIX_SLAVE = 'm';  
constexpr uint8_t TYPE_ACK          = 'a';  
constexpr uint8_t TYPE_NACK         = 'N';  

//Timeout / reintentos
constexpr int TIMEOUT_MS   = 500;
constexpr int MAX_RETRIES  = 20;

//Estructuras (packed: sin padding del compilador) 
#pragma pack(push, 1)
struct Header {
    uint16_t checksum;   
    uint16_t node_id;    
    uint16_t flags;      
    uint32_t seq;        
    uint8_t  type;       
    uint32_t data_size;  
};                       

struct Packet {
    Header  hdr;
    uint8_t data[MAX_DATA];  
};
#pragma pack(pop)

static_assert(sizeof(Header) == 15,          "Header debe ser 15 bytes");
static_assert(sizeof(Packet) == PACKET_SIZE, "Packet debe ser 500 bytes");

//Checksum
inline uint16_t compute_checksum(const Packet& p) {
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(&p);
    uint32_t sum = 0;
    for (int i = 2; i < PACKET_SIZE; ++i) sum += raw[i];
    return static_cast<uint16_t>(sum % 100);
}
inline bool verify_checksum(const Packet& p) {
    return p.hdr.checksum == compute_checksum(p);
}
inline void seal_packet(Packet& p) {
    p.hdr.checksum = compute_checksum(p);
}

} 
