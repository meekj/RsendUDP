// Example of timing Rcpp / OpenMP threads
#include <Rcpp.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <omp.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <thread>

// [[Rcpp::plugins(openmp)]]

// Version of sendUDP for embedding in C++ Rcpp code
void send_diagnostic_udp(const std::string& server_ip, 
                        int server_port, 
                        const std::string& message_string) {
    // Get high resolution timestamp and convert to nanoseconds
    auto now    = std::chrono::high_resolution_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    
    // pid_t pid = getpid(); // Process ID
    pid_t pid = omp_get_thread_num(); // OpenMP thread number
    
    // Build the message
    std::ostringstream oss;

    oss << now_ns
        << " | " << pid
        << " | " << message_string
        << "\n";
    
    std::string payload = oss.str();
    
    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }
    
    // Setup server address structure
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    // Convert IP address
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        close(sockfd);
        throw std::runtime_error("Invalid IP address format");
    }
    
    // Send the packet
    ssize_t sent = sendto(sockfd, payload.c_str(), payload.length(), 0,
                         (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    close(sockfd);
    
    if (sent < 0) {
        throw std::runtime_error("Failed to send UDP packet: " + 
                               std::string(strerror(errno)));
    }
}

using namespace Rcpp;

// Simple example, each thread sleeps for the same time and returns 1.0
// [[Rcpp::export]]
double omp_example(int num_threads_wanted, double sleep_time) {
    double res = 0.0;       // Total result will go here
    
    #pragma omp parallel num_threads(num_threads_wanted)
    {
      send_diagnostic_udp("127.0.0.1", 1800, "Start");
      double local_res;
        
      // #pragma omp single nowait
      std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
      local_res = 1.0; // Result from each thread
        
      #pragma omp critical
      {
        res += local_res; // Sum the results from each thread
      }
      send_diagnostic_udp("127.0.0.1", 1800, "End");
    }
    return res;
}


// [[Rcpp::export]]
int send_udp_cpp(std::string message) {
  send_diagnostic_udp("127.0.0.1", 1800, message);
  return 0;
}
