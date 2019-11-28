//
// Created by bassam on 28/11/2019.
//

#ifndef RDT_CLIENT_PACKETUTILS_H
#define RDT_CLIENT_PACKETUTILS_H

#include <vector>
#include "Packets.h"

using namespace std;

class PacketUtils {

public:
    static void storeFile(const string& fileName, const vector<Packets::packet>& packets);
};


#endif //RDT_CLIENT_PACKETUTILS_H
