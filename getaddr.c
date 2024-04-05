/*  This program shows an example of how to find an IP address
    associated with a computer's name (host name).
    It uses the getIPaddress() function provided in CS_TCP.c,
	which uses the library function getaddrinfo().
	This is the Linux version.  */

#include <stdio.h>		 // for input and output functions

#include <sys/socket.h>	 // Linux socket functions and structs
#include <netdb.h>	// Linux getaddrinfo() and related items
#include <arpa/inet.h>   // needed for IP address functions in some systems

//#include <winsock2.h>	 // Windows socket functions and structs
//#include <ws2tcpip.h>	 // Windows socket functions and structs

// This header file must be included AFTER the libraries above
#include "CS_TCP.h"		 // TCP functions for this assignment

int main()
{
    char serverName[80];     // string to hold server name
    char serverIPstr[20];    // server IP address as a string
    struct in_addr serverIP;    // struct to hold IP address
    unsigned long int ipAddr32;      // IP address as a 32-bit number
    int retVal;         // return value from function

    // Create a socket, to initialise the Windows sockets system (Windows only)
/*  SOCKET dummySocket = INVALID_SOCKET;  // identifier for a socket
    dummySocket = TCPcreateSocket();  // initialise and create a socket
    if (dummySocket == FAILURE)  // check for failure
        return FAILURE;       // no point in continuing  */

    // Ask user for name of host computer
    printf("\nEnter name of server (www.ucd.ie): ");
    scanf("%70s%*c", serverName);  // get name and ignore newline at end

    /* Call the function to get the IP address.  The second argument can
       be NULL if you do not want the IP address as a number.  */
    retVal = getIPaddress(serverName, &serverIP, serverIPstr);
    printf("\ngetIPaddress() returned %d\n", retVal);

    // If the result is a failure, stop now
    if (retVal != SUCCESS) return FAILURE;

    // Print the result, returned in string form (dotted decimal)
    printf("\nIP address string returned: %s\n", serverIPstr);

    /* In Windows, the in_addr struct contains a union (S_un) of other structs,
       allowing the IPv4 address to be accessed as a 32-bit number (S_addr) or
       as four separate bytes in a struct called S_un_b, with elements s_b1 etc.
       In Linux, only the 32-bit IPv4 address is available, as element s_addr
       - Windows defines s_addr as S_un.S_addr for compatibility.

       The 32-bit address is in the order required on the network: the most
       significant byte, which is sent first, is at the lowest address in memory.
       On some computers (hosts) this is the reverse of the way that values are
       normally stored.  The ntohl() and htonl() functions convert if needed. */

    // Print the result, extracted from the serverIP struct
    ipAddr32 = serverIP.s_addr;    // get the IP address from the struct
    printf("\nIP address as 32-bit number in hex (network order): %lx\n", ipAddr32);
    printf("IP address as 32-bit number in hex (host order): %lx\n", (unsigned long int) ntohl(ipAddr32));

    // Extract the four bytes of the IP address and print those in decimal
/*    printf("IP address in bytes (decimal, MSB first): %d %d %d %d\n",
           serverIP.S_un.S_un_b.s_b1, serverIP.S_un.S_un_b.s_b2,
           serverIP.S_un.S_un_b.s_b3, serverIP.S_un.S_un_b.s_b4);  */

    // Print the result as a string, generated from the serverIP struct
    printf("\nIP address converted to a string: %s\n\n", inet_ntoa(serverIP));

    // Close the socket to clean up everything (Windows only)
//    TCPcloseSocket(dummySocket);

    // Prompt for user input, so window stays open when run outside CodeBlocks
    printf("\nPress return to exit:");
    getchar();  // wait for character input

    return SUCCESS;
}

