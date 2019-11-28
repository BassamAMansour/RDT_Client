#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <cstdint>
#include "PacketUtils.h"
#include "Packets.h"

#define RECV_BUFFER_SIZE 10000

using namespace std;

void setupAddressInfo(char **argv, addrinfo **serverInfo);

int connectToServer(addrinfo **serverInfo);

void requestFile(int socketFd, const string &fileName);

void sendPacket(int socketFd, Packets::packet requestPacket);

vector<Packets::packet> receiveFile(int socketFd, int packetsCount, unsigned int initialSeqNo);

Packets::packet receivePacket(int socketFd);

void printReceivedPacket(Packets::packet packet);

void sendAck(int socketFd, uint32_t sequenceNumber);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: client hostname, port and filename\n");
        exit(1);
    }

    struct addrinfo *serverInfo;

    setupAddressInfo(argv, &serverInfo);
    int socketFd = connectToServer(&serverInfo);

    requestFile(socketFd, string(argv[3]));

    freeaddrinfo(serverInfo);
    close(socketFd);

    return 0;
}

void requestFile(int socketFd, const string &fileName) {
    uint16_t requestSize = Packets::PACKET_HEADER_SIZE + fileName.size();

    struct Packets::packet fileRequestPacket{0, requestSize, 1, ' '};
    copy(fileName.begin(), fileName.end(), fileRequestPacket.data);

    sendPacket(socketFd, fileRequestPacket);

    Packets::packet fileInfoPacket = receivePacket(socketFd);

    vector<Packets::packet> receivedPackets = receiveFile(socketFd, stoi(fileInfoPacket.data),
                                                          fileInfoPacket.seqno + 1);

    PacketUtils::storeFile(fileName, receivedPackets);
}

Packets::packet receivePacket(int socketFd) {
    char buffer[RECV_BUFFER_SIZE + 1];
    int receivedBytes;

    printf("client: waiting to recv...\n");

    if ((receivedBytes = recv(socketFd, buffer, RECV_BUFFER_SIZE, 0)) == -1) {
        perror("recv");
        exit(1);
    } else if (receivedBytes == 0) {
        printf("Received 0 bytes. Calling receive packet function again.\n");
        return receivePacket(socketFd);
    }

    struct Packets::packet receivedPacket{};

    memcpy(&receivedPacket, buffer, receivedBytes);

    printReceivedPacket(receivedPacket);

    sendAck(socketFd, receivedPacket.seqno);

    return receivedPacket;
}

vector<Packets::packet> receiveFile(int socketFd, int packetsCount, unsigned int initialSeqNo) {

    Packets::packet packets[packetsCount];

    int receivedPackets = 0;

    while (receivedPackets < packetsCount) {
        Packets::packet receivedPacket = receivePacket(socketFd);
        packets[receivedPacket.seqno - initialSeqNo] = receivedPacket;
        receivedPackets++;
    }

    return vector<Packets::packet>(packets, packets + packetsCount);
}

void sendAck(int socketFd, uint32_t sequenceNumber) {
    sendPacket(socketFd, Packets::packet{0, Packets::ACK_PACKET_SIZE, sequenceNumber});
    printf("Sent ack with seq#: %d\n", sequenceNumber);
}

void sendPacket(int socketFd, Packets::packet requestPacket) {
    int requestSize = requestPacket.len;
    char buffer[requestSize];
    memcpy(buffer, (const unsigned char *) &requestPacket, requestSize);

    int sentBytes = 0;
    if ((sentBytes = send(socketFd, buffer, requestSize, 0)) == -1) {
        perror("client: sendto");
        exit(1);
    }

    if (sentBytes != requestSize) {
        printf("client: error sending the file. Sent bytes %d out of %d. Calling the function again.",
               sentBytes, requestSize);

        sendPacket(socketFd, requestPacket);
    }
}

int connectToServer(addrinfo **serverInfo) {
    int socketFd = -1;
    struct addrinfo *addressIterator;

    for (addressIterator = *serverInfo; addressIterator != nullptr; addressIterator = addressIterator->ai_next) {

        if ((socketFd = socket(addressIterator->ai_family, addressIterator->ai_socktype,
                               addressIterator->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (connect(socketFd, addressIterator->ai_addr, addressIterator->ai_addrlen) == -1) {
            perror("client: connect");
            close(socketFd);
            continue;
        }
    }

    if (socketFd == -1) {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    printf("Created socket #%d\n", socketFd);

    return socketFd;
}

void setupAddressInfo(char **argv, addrinfo **serverInfo) {
    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(argv[1], argv[2], &hints, serverInfo);
    if ((status) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
}

void printReceivedPacket(Packets::packet packet) {
    printf("Packet: checkSum-> %d , length-> %d , seq#-> %d\n", packet.cksum, packet.len, packet.seqno);

    if (packet.len > Packets::ACK_PACKET_SIZE) {
        printf("Packet: data-> %s\n", packet.data);
    }
}