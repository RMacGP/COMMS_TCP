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
#include <time.h>

#include <sys/socket.h>	 // Linux socket functions and structs
#include <netdb.h>		// Linux getaddrinfo() and related items
#define INVALID_SOCKET SOCKET_ERROR  // fix an incompatibility

//#include <winsock2.h>	 // Windows socket functions and structs
//#include <ws2tcpip.h>	 // Windows socket functions and structs

// This header file must be included AFTER the libraries above
#include "CS_TCP.h"		 // TCP functions for this assignment

// Marker used in this example protocol
#define ENDMARK 10       // the newline character
#define CR 13       // the carriage return character

// Size limits - these should be much bigger for a real application
#define MAXREQUEST 250    // maximum size of request, in bytes
#define MAXRESPONSE 15000   // size of response array, in bytes
#define MAXHEADER 9000 // most HTTP servers have a cap of ~8KB on response headers
                       // https://httpd.apache.org/docs/2.2/mod/core.html#limitrequestfieldsize


// Create variables needed by this function
// SOCKET clientSocket = INVALID_SOCKET;  // identifier for the client socket, Windows

char fullWebAddress[100];        // Character web address the user inputs
char webAddress[100];
char serverIPstr[100];       // IP address of server as a string
char webDirectory[100];     // Directory from root of ip address in server
char fileSaveName[100];      // Name of the saved file
int serverPort = 80;             // server port number
char request[MAXREQUEST+1];   // array to hold request from user
char response[MAXRESPONSE+1]; // array to hold response from server
char header[MAXHEADER+1];


/*
http://home.datacomm.ch/t_wolf/tw/c/getting_input.html
*/
char *read_line (char *buf, size_t length, FILE *f)
  /**** Read at most 'length'-1 characters from the file 'f' into
        'buf' and zero-terminate this character sequence. If the
        line contains more characters, discard the rest.
   */
{
  char *p;

  if (p = fgets (buf, length, f)) {
    size_t last = strlen (buf) - 1;

    if (buf[last] == '\n') {
      /**** Discard the trailing newline */
      buf[last] = '\0';
    } else {
      /**** There's no newline in the buffer, therefore there must be
            more characters on that line: discard them!
       */
      fscanf (f, "%*[^\n]");
      /**** And also discard the newline... */
      (void) fgetc (f);
    } /* end if */
  } /* end if */
  return p;
} /* end read_line */

int downloadHttpFile()
{

  SOCKET clientSocket = SOCKET_ERROR;  // identifier for the client socket, Linux
  struct in_addr server_in_addr;      // struct to hold the IP address of the server
  int retVal;                 // return value from various functions
  int numRx;                  // number of bytes received this time
  int numBytes = 0;           // number of bytes received in total
  int reqLen;                 // request string length
  int stop = 0;               // flag to control the loop
  int contentLength = -1;
  int headerLength = 0;
  // ============== CONNECT TO SERVER =============================================

  clientSocket = TCPcreateSocket();  // initialise and create a socket
  if (clientSocket == FAILURE)  // check for failure
    return 1;       // no point in continuing

  // scanf("%100s", fullWebAddress);  // get user web address as a string

  char *httpFound = strstr(fullWebAddress, "http://"); // excluding https !
  int startOfWebAddress = 0;
  if (httpFound != NULL && httpFound == fullWebAddress)
  {
    startOfWebAddress = 7;
  }
  strcpy(webAddress, fullWebAddress+startOfWebAddress);
  
  // maybe implement another check to filter out users trying to use https
  char* findSlash = strchr(webAddress+startOfWebAddress, '/'); // find first instance of / after  // get rid of startOfWebaddress
  if (findSlash == NULL)
  {
    webDirectory[0] = '/';
    webDirectory[1] = '\0';
  }
  else
  {
    strncpy(webDirectory, findSlash, strlen(webAddress)-(findSlash-webAddress));
    webDirectory[strlen(webAddress)-(findSlash-webAddress)] = '\0';
    webAddress[findSlash-webAddress] = '\0';
  }

  //  printf("Please enter the directory of this address you would like to retrieve: ");

  // scanf("%100s", webDirectory);

  //  printf("%s", webAddress);
  //  printf("\n%s\n", webDirectory);

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
    
    // char request[] = "GET /~grovesd/images/big-bear.png HTTP/1.1\r\nHost: www.web.simmons.edu\r\n\r\n";
    snprintf(request, MAXREQUEST, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", webDirectory, webAddress);

    printf("REQUEST: %s", request);
    
    reqLen = strlen(request);
    retVal = send(clientSocket, request, reqLen, 0);  // send bytes
    // retVal will be the number of bytes sent, or a problem indicator

    if( retVal == SOCKET_ERROR) // check for problem
    {
      printf("*** Problem sending\n");
      printProblem();   // print the details
      stop = 1;       // set the flag to skip to tidy-up section
    }
    else
      printf("Sent request with %d bytes, waiting for reply...\n", retVal);
  }


// ============== RECEIVE RESPONSE ======================================

  FILE *outFile, *headerFile;
  outFile = fopen(fileSaveName, "wb");

  /* Loop to receive the entire response - it could be long!  This
  loop ends when the end of response string is found in the response,
  or when the server closes the connection, or when a problem occurs. */
  int endOfHeaderIdx = 0;
  while (stop == 0)
  {

    /* Wait to receive bytes from the server, using the recv function.
    recv() arguments: socket identifier, array to hold received bytes,
    maximum number of bytes to receive, last argument 0.
    Normally, recv() will not return until it receives at least one byte */
    numRx = recv(clientSocket, response, MAXRESPONSE, 0);
    // numRx will be number of bytes received, or a problem indcator

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

      if (numBytes == 0) // first iteration of loop, we should find header end marker
      {
        response[MAXRESPONSE] = '\0';
        char *endmarker = strstr(response, "\r\n\r\n");
        char *findContentLenght = strstr(response, "Content-Length:");
        if (endmarker == NULL || findContentLenght == NULL)
        {
          printf("\n*** Did not receive an appropriate header\n");
          stop = 1;
          break;
        }
        headerLength = (endmarker+4) - response;
        if (sscanf(findContentLenght+15, " %i\n", &contentLength) != 1)
        {
          printf("\n*** Could not extract content-length from header\n");
          stop = 1;
          break;
        }
      }
      

      for (int i = (numBytes == 0 ? headerLength : 0); i<numRx; i++)
      {
        fwrite(&response[i], 1, sizeof(response[i]), outFile);
      }
      numBytes += numRx;    // add to byte counter
  
      // check if the client should close the connection early before attempting to receive bytes
      if (numBytes != 0 && numBytes - headerLength >= contentLength) // we have already received the amount of bytes indicated in the hdeader 
      {
        printf("\nDetected that all bytes have been received.\n");
        stop = 1;
      }

      // // Print whatever has been received
      // response[numRx] = 0; // convert the response array to a string
      // // The array was made larger to leave room for this extra byte.

      // printf("\nReceived %d bytes from the server:\n|%s|\n", numRx, response);
      // /* Note the response is printed between bars,
      // to make it easy to see where it begins and ends. */
    } // end of if (numRx > 0)
  }   // end of while loop - repeat if data received but end not found
  
  // ============== TIDY UP AND END ======================================

  /* A better client might loop to allow another request to be sent.
  This simple client just stops after the full response has been
  received from the server. */

  printf("\nClient is closing the connection...\n");

  // Close the socket and shut down the WSA system
  TCPcloseSocket(clientSocket);

  return SUCCESS;
} 

int main()
{

  // Print starting message
  printf("\nCommunication Systems client program\n\n");
  
  char yesOrNo[2];
  do
  {
    // Get the web address from the user
    printf("\nEnter the full web address you  would like to access (maximum %d bytes): ", MAXREQUEST-2);
    read_line(fullWebAddress, MAXREQUEST, stdin);
    printf("Please enter the name of the file to save as: ");
    read_line(fileSaveName, 100, stdin);

    downloadHttpFile();
    printf("Your downloaded file has been saved to this directory as %s! To download another file enter 'y', otherwise enter any other character to end: ", fileSaveName);
    read_line(yesOrNo, 2, stdin);
  }while (yesOrNo[0] == 'y');

  printf("\nSee you soon !!\n\n");

  return 0;
}
