# RsendUDP
Send a message in a UDP packet from R

Sends a message in a single UDP packet for debugging and diagnostic purposes.
The original use was to debug parallel processing in R and, using a similar function (to be included here later), 
C++ OpenMP code. 

### To install:

	library(devtools)
	devtools::install_github("meekj/RsendUDP")

### Example Usage from R

    library(RsendUDP)
    library(stringr)

    message_string <- str_c(Sys.time(), ' function_name ', Sys.getpid(), ' A diagnostic message\n')
    sendUDP_stat <- sendUDP("127.0.0.1", 1800, message_string)

127.0.0.1 is the localhost IP address, the IP address of another system on your network could also be used
1800 is the port number

sendUDP returns zero if the packet was sent successfully.

Collect messages in a Linux terminal:

    nc -lku 1800

nc on MacOS prints only the first packet, use ncat (from nmap) instead:

    ncat -lku 1800

This version of sendUDP probably does not work on Windows.

