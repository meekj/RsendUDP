#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <Rcpp.h>


//' Send a message in a UDP packet from R. Returns status, 0 = no error
//'
//' @export
// [[Rcpp::export]]
int sendUDP(std::string serverIP, int port, std::string message) {
    // --- Create UDP Socket ---
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        Rcpp::Rcerr << "Failed to create socket." << std::endl;
        return 1;
    }

    // --- Define Target Address and Port ---
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert IP address string to network address structure
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        Rcpp::Rcerr << "Invalid IP address: " << serverIP << std::endl;
        close(sock);
        return 1;
    }

    // --- Send the UDP Packet ---
    int bytesSent = sendto(sock, message.c_str(), message.length(), 0,
                           (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if (bytesSent == -1) {
        Rcpp::Rcerr << "Failed to send message." << std::endl;
        close(sock);
        return 1;
    }

    // Rcpp::Rcout << "Sent " << bytesSent << " bytes to " << serverIP << ":" << port << std::endl;

    close(sock);
    return 0;
}

