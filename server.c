/*  EEEN20060 Communication Systems
    Simple TCP server program, to demonstrate the basic concepts,
    using IP version 4.  This is the Linux version.

    The server listens on a specified port until a client connects.
    Then it waits for a request from the client.  As the request
    could be long, it is received in sections and assembled in
    an array.  This continues until the end of the request, marked
    by 2 consecutive end markers.
    Then the server sends a response to the client, in three parts.
    The first and third parts are fixed text, but the second part
    includes a copy of the request, just as a demonstration.
    When the response has been sent, the server closes the connection
    to the client - in some applications, it might be better to
    keep the connection open.
    Then the server loops back to listen for another connection.

    This program is not robust - if a problem occurs, it just
    tidies up and exits, with no attempt to fix the problem.  */

#include <stdio.h>          // input and output functions
#include <string.h>         // string handling functions

#include <sys/socket.h>	 // Linux socket functions and structs
#include <netdb.h>		// Linux getaddrinfo() and related items
#define INVALID_SOCKET SOCKET_ERROR  // fix an incompatibility

//#include <winsock2.h>	 // Windows socket functions and structs
//#include <ws2tcpip.h>	 // Windows socket functions and structs

// This header file must be included AFTER the libraries above
#include "CS_TCP.h"		 // TCP functions for this assignment

// Port number - this port has been opened in the firewall on the lab PCs
#define SERVER_PORT 32980  // port to be used by the server

// Markers used for this example protocol
#define ENDMARK 10    // the newline character  is the end marker
#define CR 13     // the CR character is ignored when counting end markers

// Size limits - these should all be much bigger for a real application
#define MAXRECEIVE   20    // maximum number of bytes to receive at a time
#define MAXREQUEST  100    // size of request array, in bytes
#define MAXRESPONSE 150    // size of response array (at least 46 bytes more)


int main()
{
  // Create variables needed by this function
  /* The server uses 2 sockets - one to listen for connection requests,
     the other to make a connection with the client. */
  SOCKET listenSocket = INVALID_SOCKET;  // identifier for listening socket
  SOCKET connectSocket = INVALID_SOCKET; // identifier for connection socket
  int retVal;         // return value from various functions
  int numRx;          // number of bytes received this time
  int totalReq;       // number of bytes received in this request
  int numRecv;        // number of times recv() has got data on this connection
  int numResp;        // number of bytes in response string
  int totalResp;      // total number of bytes sent as response
  enum stopValues {CONTIN, CLOSE, FINAL}; // values for stop variable
  int stop = CONTIN;       // variable to control the loops
  int endCounter;     // count of consecutive end markers
  int i;              // used in for loop
  char request[MAXREQUEST+1];   // array to hold request from client
  char response[MAXRESPONSE+1]; // array to hold our response
  char welcome[] = "Welcome to the Communication Systems server.\r\n";
  char goodbye[] = "Goodbye and thank you for using the server. ###";

  // Print starting message
  printf("\nCommunication Systems server program\n\n");


  // ============== SERVER SETUP ===========================================

  listenSocket = TCPcreateSocket();  // initialise and create a socket
  if (listenSocket == FAILURE)  // check for problem
    return 1;       // no point in continuing

  // Set up the server to use this socket and the specified port
  retVal = TCPserverSetup(listenSocket, SERVER_PORT);
  if (retVal == FAILURE) // check for problem
    stop = FINAL;   // set the flag to prevent other things happening


  // ============== LOOP TO HANDLE CONNECTIONS =============================

  while (stop != FINAL)
  {
    stop = CONTIN;  // initialise the stop signal for this connection
    numRecv = 0;    // initialise the receive counter
    totalReq = 0;   // initialise number of bytes in the request
    endCounter = 0; // initialise the end marker counter
    totalResp = 0;  // initialise the response byte counter


    // ============== WAIT FOR CONNECTION ====================================

    // Listen for a client to try to connect, and accept the connection
    connectSocket = TCPserverConnect(listenSocket);
    if (connectSocket == FAILURE)  // check for problem
      stop = CLOSE;   // prevent other things happening


    // ============== RECEIVE REQUEST ==================================

    // Loop to receive data from the client, until the request is complete
    while (stop == CONTIN)   // loop is controlled by the stop variable
    {
      printf("Waiting for data from the client...\n");

      /* Wait to receive bytes from the client, using the recv() function
         recv() arguments: socket identifier, pointer to array for received bytes,
         maximum number of bytes to receive, last argument 0.  Normally,
         this function will not return until it receives at least one byte. */
      numRx = recv(connectSocket, request+totalReq, MAXRECEIVE, 0);
      // numRx will be number of bytes received, or a problem indicator

      if( numRx < 0)  // check for problem
      {
        printf("\nProblem receiving\n");
        printProblem();   // give details of the problem
        stop = CLOSE;     // this will end the inner loop
      }
      else if (numRx == 0)  // indicates connection closing (normal close)
      {
        printf("\nConnection closed by client\n");
        stop = CLOSE;    // this will end the inner loop
      }
      else    // numRx > 0 => we got some data from the client
      {
        numRecv++;   // increment the receive counter
        printf("Received %d bytes from the client\n", numRx);

        /* Check if the client has completed the request.
           The end of a request is marked by two consecutive end markers,
           with nothing between them except possibly CR bytes.
           This method of counting end markers will find two consecutive
           markers even if they arrive in separate calls to recv() */
        for (i=totalReq; i<totalReq+numRx; i++) // for each new received byte
        {
          if (request[i] == ENDMARK) // if end marker found
            endCounter++;  // increment counter
          else if (request[i] != CR)  // if anything else except CR
            endCounter = 0; // reset the counter
          if (endCounter == 2)    // found 2 consecutive markers
          {
            stop = CLOSE;   // end of request found, can stop receiving
            printf("\nFound the end marker after %d bytes\n", i+1);
          }
        } // end of for loop

        totalReq += numRx;  // update the number of bytes in the request

        /* If the end marker has not been found yet, check if there is
           enough room in the request array to try to receive more data.
           This is a crude solution, and will often stop before the array is full. */
        if (stop == CONTIN && totalReq+MAXRECEIVE > MAXREQUEST)
        {
          stop = CLOSE;   // stop receiving, array is close to full
          printf("No end marker, approaching size limit, %d bytes received\n", totalReq);
        }
      }  // end of else - finished processing the bytes received this time

    }  // end of inner while loop, receiving request

    // Print the request at the server, for debug purposes
    request[totalReq] = 0;  // add 0 byte to make request into a string
    printf("\nReceived total of %d bytes: |%s|\n", totalReq, request);
    /* Note the request is printed between bars,
       to make it easy to see where it begins and ends. */


    // ============== SEND RESPONSE ==================================

    // The response has three parts in this example: a welcome message,
    // a copy of the request and a closing message.

    // First send the welcome message
    /* send() arguments: socket identifier, array of bytes to send,
       number of bytes to send, and last argument of 0.  */
    retVal = send(connectSocket, welcome, strlen(welcome), 0);
    // retVal will be the number of bytes sent, or a problem indicator

    if( retVal == SOCKET_ERROR)  // check for problem
    {
      printf("\n*** Problem sending welcome message\n");
      printProblem();
      stop = FINAL;   // set the flag to end the outer loop
      continue;       // skip the rest of the response
    }
    else
    {
      printf("\nSent welcome message of %d bytes\n", retVal);
      totalResp += retVal;
    }

   /* Build the next part of the response as a string, including the
       the request, which is between brackets for clarity.
       snprintf() works like printf(), but puts the result in a string,
       up to a maximum number of bytes.  The return value is the number
       of bytes that it has put into the string. */
    numResp = snprintf(response, MAXRESPONSE-1,
            "Got your request with %d bytes: [%s]\r\n\r\n",
            totalReq, request);

    // Send the response to the client (without string terminator)
    retVal = send(connectSocket, response, numResp, 0);
    if( retVal == SOCKET_ERROR)  // check for problem
    {
      printf("*** Problem sending response part 2\n");
      printProblem();
      stop = FINAL;   // set the flag to end the outer loop
      continue;       // skip the rest of the response
    }
    else
    {
      printf("Sent response of %d bytes\n", retVal);
      totalResp += retVal;
    }

    // Send the closing message
    retVal = send(connectSocket, goodbye, strlen(goodbye), 0);
    if( retVal == SOCKET_ERROR)  // check for problem
    {
      printf("*** Problem sending closing message\n");
      printProblem();
      stop = FINAL;   // set the flag to end the loop
    }
    else
    {
      printf("Sent closing message of %d bytes\n", retVal);
      totalResp += retVal;
    }

    // Print some information at the server
    printf("\nFinished processing request from client\n");
    printf("Sent a total of %d bytes to the client\n", totalResp);


    // ============== CLOSE THE CONNECTION ======================================

    /* A better server might loop to deal with another request from the client.
    This one disconnects after dealing with one request. */
    printf("\nServer is closing the connection...\n");

    // Close the connection socket
    TCPcloseSocket(connectSocket);

  } // end of outer while loop - go back to wait for another connection


  // ================== TIDY UP AND SHUT DOWN THE SERVER ==========================

  // If we get to this point, something has gone wrong

  //Close the listening socket
  TCPcloseSocket(listenSocket);

  // Prompt for user input, so window stays open when run outside CodeBlocks
  printf("\nPress enter key to end:");
  getchar();  // wait for character input

  return SUCCESS;
}
