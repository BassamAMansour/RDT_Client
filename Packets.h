//
// Created by bassam on 27/11/2019.
//

#ifndef RDT_SERVER_PACKETS_H
#define RDT_SERVER_PACKETS_H

#include <cstdint>

using namespace std;

class Packets {

public:

    static const int MAX_DATA_SIZE = 500;

    struct packet {
        uint16_t cksum;
        uint16_t len;
        uint32_t seqno;
        char data[MAX_DATA_SIZE];
    };

    struct ack_packet {
        uint16_t cksum;
        uint16_t len;
        uint32_t seqno;
    };

    static const int ACK_PACKET_SIZE = sizeof(ack_packet);
    static const int PACKET_HEADER_SIZE = ACK_PACKET_SIZE;
};


#endif //RDT_SERVER_PACKETS_H
