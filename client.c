/*  EEEN20060 Communication Systems
    Simple TCP client program, to demonstrate the basic concepts,
    using IP version 4.  This is the Linux version.

    The client program asks for details of the server, and tries
    to connect.  Then it gets a request string from the user,
    and sends this to the server.  Then it waits for a response
    from the server, which could be quite long, so it is received
    in sections.  This continues until the string ### is found
    in the server response, or the server closes the connection.
    Then the client program tidies up and exits.

    This program is not robust - if a problem occurs, it just
    tidies up and exits, with no attempt to fix the problem.  */

#include <stdio.h>		 // for input and output functions
#include <string.h>	     // for string handling functions

#include <sys/socket.h>	 // Linux socket functions and structs
#include <netdb.h>		// Linux getaddrinfo() and related items
#define INVALID_SOCKET SOCKET_ERROR  // fix an incompatibility

//#include <winsock2.h>	 // Windows socket functions and structs
//#include <ws2tcpip.h>	 // Windows socket functions and structs

// This header file must be included AFTER the libraries above
#include "CS_TCP.h"		 // TCP functions for this assignment

// Marker used in this example protocol
#define ENDMARK 10       // the newline character

// Size limits - these should be much bigger for a real application
#define MAXREQUEST 80    // maximum size of request, in bytes
#define MAXRESPONSE 1000   // size of response array, in bytes


int main()
{
  // Create variables needed by this function
// SOCKET clientSocket = INVALID_SOCKET;  // identifier for the client socket, Windows
  SOCKET clientSocket = SOCKET_ERROR;  // identifier for the client socket, Linux
  int retVal;                 // return value from various functions
  int numRx;                  // number of bytes received this time
  int numBytes = 0;           // number of bytes received in total
  int reqLen;                 // request string length
  int stop = 0;               // flag to control the loop
  char * loc = NULL;          // location of string
  char webAddress[30];        // Character web address the user inputs
  struct in_addr server_in_addr;      // struct to hold the IP address of the server
  char serverIPstr[30];       // IP address of server as a string
  char web_directory[30];     // Directory from root of ip address in server
  char full_web_address[60];  // Web address + web directory
  int serverPort = 80;             // server port number
  char request[MAXREQUEST+1];   // array to hold request from user
  char response[MAXRESPONSE+1]; // array to hold response from server
  char endResponse[] = "###";   // string used as end marker in response

  // Print starting message
  printf("\nCommunication Systems client program\n\n");


// ============== CONNECT TO SERVER =============================================

  clientSocket = TCPcreateSocket();  // initialise and create a socket
  if (clientSocket == FAILURE)  // check for failure
    return 1;       // no point in continuing

  /*
  // Get the details of the server from the user
  printf("\nEnter the IP address of the server (e.g. 137.43.168.123): ");
  scanf("%20s", serverIPstr);  // get IP address as a string`
  */
  
  // Get the web address from the user
  printf("\nEnter the web address you wish to access (e.g. http://web.simmons.edu/~grovesd/images/big-bear.png): ");

  scanf("%30s", webAddress);  // get user web address as a string

  printf("Please enter the directory of this address you would like to retrieve: ");

  scanf("%30s", web_directory);

  printf("%s", webAddress);
  printf("\n%s\n", web_directory);

  strcat(full_web_address, webAddress);
  if (web_directory[0] != '/')
    strcat(full_web_address, "/");
  strcat(full_web_address, web_directory);

  printf("%s\n", full_web_address);

  retVal = getIPaddress(webAddress, NULL, serverIPstr);
  if (retVal == FAILURE)
    return 1;

  /*
  printf("\nEnter the port number on which the server is listening: ");
  scanf("%d", &serverPort);     // get port number as an integer
  fgets(request, MAXREQUEST, stdin);  // clear the input buffer
  */

  // Now connect to the server
  retVal = TCPclientConnect(clientSocket, serverIPstr, serverPort);
  if (retVal < 0)  // failed to connect
    stop = 1;    // set the flag so skip directly to tidy-up section

// ============== SEND REQUEST ======================================

  if (stop == 0)      // if we are connected
  {
    // Get user request and send it to the server

    printf("\nEnter request (maximum %d bytes): ", MAXREQUEST-2);
    fgets(request, MAXREQUEST, stdin);  // read in request string
    /* The fgets() function reads characters until the enter key is
    pressed or the limit is reached.  If enter is pressed, the
    newline (\n) character is included in the string.  The null
    character is also included to mark the end of the string.
    The request array needs space for both of these, so the user
    is told not to enter more than MAXREQUEST-2 characters.  */

    reqLen = strlen(request);  // find the length of the request

    /* Put two end markers at the end of the request.  The array
    was made large enough to allow an extra character to be added.  */
    request[reqLen-1] = ENDMARK;  // end marker in last position
    request[reqLen] = ENDMARK;  // add another end marker
    reqLen++; // the string is now one byte longer
    request[reqLen] = 0;   // add a new end of string marker

    /* Now send the request to the server over the TCP connection.
    send() arguments: socket identifier, array of bytes to send,
    number of bytes to send, and last argument of 0.  */
    
    // char request[] = "GET /~grovesd/images/big-bear.png\r\nHost: www.web.simmons.edu\r\n\r\n";
    char request[100];
    snprintf(request, 100, "GET /%s\r\nHost: %s\r\n\r\n", web_directory, webAddress);
    // strcat(request, web_directory);
    // strcat(request, "\r\nHost: ");
    // strcat(request, webAddress);
    // strcat(request, "\r\n\r\n");
    printf("%s\n",request);
    reqLen = strlen(request);

    printf("reqlen = %d\n", reqLen);
    retVal = send(clientSocket, request, reqLen, 0);  // send bytes
    // retVal will be the number of bytes sent, or a problem indicator

    if( retVal == SOCKET_ERROR) // check for problem
    {
      printf("*** Problem sending\n");
      printProblem();   // print the details
      stop = 1;       // set the flag to skip to tidy-up section
    }
    else printf("Sent request with %d bytes, waiting for reply...\n", retVal);
  }


// ============== RECEIVE RESPONSE ======================================

  FILE *fptr;
  fptr = fopen("request_rx.txt", "ab+");

  /* Loop to receive the entire response - it could be long!  This
  loop ends when the end of response string is found in the response,
  or when the server closes the connection, or when a problem occurs. */
  while (stop == 0)
  {
    /* Wait to receive bytes from the server, using the recv function.
    recv() arguments: socket identifier, array to hold received bytes,
    maximum number of bytes to receive, last argument 0.
    Normally, recv() will not return until it receives at least one byte */
    numRx = recv(clientSocket, response, MAXRESPONSE, 0);
    // numRx will be number of bytes received, or a problem indicator

    if( numRx == SOCKET_ERROR)  // check for problem
    {
      printf("\n*** Problem receiving\n");
      printProblem();   // print details of the problem
      stop = 1;  // set flag to exit the loop
    }
    else if (numRx == 0)  // connection closed
    {
      printf("\nConnection closed by server\n");
      stop = 1;
    }
    else if (numRx > 0)  // we got some data from the server
    {
      numBytes += numRx;    // add to byte counter
      for (int i=0; i<numRx; i++) // yo where is the header
      {
        fwrite(&response[i], 1, sizeof(response[i]), fptr);
      }
      // Print whatever has been received
      response[numRx] = 0; // convert the response array to a string
      // The array was made larger to leave room for this extra byte.
      printf("\nReceived %d bytes from the server:\n|%s|\n", numRx, response);
      /* Note the response is printed between bars,
      to make it easy to see where it begins and ends. */

      /* Check if the end of response string has been received.
      This method is not completely safe - it will fail if the string
      is split between two blocks of received bytes...  */
      loc = strstr(response, endResponse);  // returns pointer to string if found
      if (loc != NULL)
      {
        printf("\nEnd of response marker found\n");
        printf("Received a total of %d bytes from the server\n", numBytes);
        stop = 1;	// set the flag to exit the loop
      }
    } // end of if (numRx > 0)

  }   // end of while loop - repeat if data received but end not found
  fclose(fptr);


// ============== TIDY UP AND END ======================================

  /* A better client might loop to allow another request to be sent.
  This simple client just stops after the full response has been
  received from the server. */

  printf("\nClient is closing the connection...\n");

  // Close the socket and shut down the WSA system
  TCPcloseSocket(clientSocket);

  // Prompt for user input, so window stays open when run outside CodeBlocks
  printf("\nPress enter key to end:");
  getchar();  // wait for character input

  return SUCCESS;
}
