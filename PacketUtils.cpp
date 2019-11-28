//
// Created by bassam on 28/11/2019.
//

#include <string>
#include <fstream>
#include "PacketUtils.h"

void PacketUtils::storeFile(const string& fileName, const vector<Packets::packet>& packets) {
    ofstream outFile(string("./").append(fileName), ios::binary);

    for (auto packet:packets) {
        outFile <<  string(packet.data, packet.len - Packets::PACKET_HEADER_SIZE) << endl;
    }

    outFile.close();
}
