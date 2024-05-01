/*  EEEN20060 Communication Systems
    Group 26
      Ruadhán Mac Giolla Phádraig 22440322
      Caleb Ryan 22432254
    The program:
      - Gets user input of web address url, and the file name to save as
      - Connects to the server after processing the user input and retrieving the ip address
      - Sends a request built from the user input
      - Processes the response from the server, including finding the content length from the header
      - saves the file data after header to file with user inputted filename
      - closes file, and goes back to first bullet point if user enters 'y' to go again
*/

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
#define CR 13       // the carriage return character

// Size limits - these should be much bigger for a real application
#define MAXREQUEST 250    // maximum size of request, in bytes
#define MAXRESPONSE 15000   // size of response array, in bytes
#define MAXHEADER 9000 // most HTTP servers have a cap of ~8KB on response headers
                       // https://httpd.apache.org/docs/2.2/mod/core.html#limitrequestfieldsize


// Create global variables needed by the program
// Their values need not be cleared every time we run downloadHttpFile again
struct in_addr server_in_addr;// struct to hold the IP address of the server
int retVal;                   // return value from various functions
int numRx;                    // number of bytes received this time
char fullWebAddress[100];     // array to hold full web address inputted by user as a string
char webAddress[100];         // array to contain the web address excluding http:// prefix and stopping after top-level domain (.ie/.com) as a string
char serverIPstr[100];        // array to hold IP address of server as a string
char webDirectory[100];       // array to hold directory and file of GET request on server as a string (everyhting after top-level domain)
char fileSaveName[100];       // array to hold user inputted name of file as a string
int serverPort = 80;          // server port number (80 for http)
char request[MAXREQUEST+1];   // array to hold request from user
char response[MAXRESPONSE+1]; // array to hold response from server
char header[MAXHEADER+1];     // array to hold http response header from server


/*
http://home.datacomm.ch/t_wolf/tw/c/getting_input.html
This function overcomes all challenges posed by scanf and fgets including flushing stdin
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
  /*  Called from main() to 
    - Process user input (get webAddress / directory)
    - Create http request
    - send http request
    - process http respose
    - save http response to file
  */
  // These variables were not declared globally as their starting value can be important
  SOCKET clientSocket = SOCKET_ERROR;  // identifier for the client socket, Linux
  int numBytes = 0;           // number of bytes received in total
  int reqLen;                 // request string length
  int stop = 0;               // flag to control the loop
  int contentLength = -1;
  int headerLength = 0;
  // ============== CONNECT TO SERVER =============================================

  clientSocket = TCPcreateSocket();  // initialise and create a socket
  if (clientSocket == FAILURE)  // check for failure
    return 1;       // no point in continuing

  char *httpFound = strstr(fullWebAddress, "http://");    // pointer to first occurance of "http://" in fullWebAdress, returns NULL if not found
  int startOfWebAddress = 0;                              // index of where web address starts after http:// It has value 0 if http:// is not at start, and 7 if it is
  if (httpFound != NULL && httpFound == fullWebAddress)   // if it is found AND equal to fullWebAddress (at the start of string, this is a pointer to the first character in the string)
  {
    startOfWebAddress = 7;                                // Since we found http:// at the start, we set the index startOfWebAddress to 7 so http:// can be skipped later on
  }
  // webAddress stores the address without 'http://'. If it wasn't there to begin with then it will be the same as fullWebAddress
  strcpy(webAddress, fullWebAddress+startOfWebAddress);   // We are copying the string (fullWebAddress+startOfWebAddress) into webAddress.
  // fullWebAddress is a pointer to the first element, and startOfWebAddress offsets the pointer by 7 if 'http://' was in fullWebaddress otherwise by 0 (it doesn't offset it)
  
  // findSlash is a pointer to the first occurance of a '/' char in webAddress. It is NULL otherwise
  char* findSlash = strchr(webAddress, '/'); // find first instance of '/', remember this string will not contain 'http://' anymore
  if (findSlash == NULL)                                  // We did not find a slash
  {
    /*  webDirectory stores everything after the TLD (top-level domain). In the case where the user 
        did not enter a directory (eg. faraday1.ucd.ie does not have directory specified whereas
        faraday.ucd.ie/contact.html does), then we set webDirectory to '/'. Why? because for the
        purpose of our http request, it will need to be GET / HTTP/1.1, where the slash between
        GET and HTTP/1.1 is webDirectory. */
    webDirectory[0] = '/';
    webDirectory[1] = '\0';                               // Terminating character to make it a string
  }
  else                                                    // We did find a slash
  {
    /*strncpy allows us to copy n characters from one string into another. In our case we are copying everything from and including the slash to the right from webAddress into webDirectory
      findSlash is a pointer to the forward slash after the TLD, and directoryLength [strlen(webAddress)-(findSlash-webAddress)] is the number of characters including the slash to the end of webAddress
      So, we copy directoryLength characters from the findSlash pointer of webAddress into webDirectory */
    int directoryLength = strlen(webAddress) - (findSlash-webAddress);
    strncpy(webDirectory, findSlash, directoryLength);
    webDirectory[directoryLength] = '\0';                 // strncpy does not add a terminating character so we do this to make it a string
    webAddress[findSlash-webAddress] = '\0';              // we can now truncate the directory part of webAddress by adding the terminating character where we found the first slash
  }

  retVal = getIPaddress(webAddress, NULL, serverIPstr);    // We try and get the IP address using webAddress
  if (retVal == FAILURE)                                   // Did not succeed in finding it, return 1.
    return 1;

  // Now connect to the server
  retVal = TCPclientConnect(clientSocket, serverIPstr, serverPort);
  if (retVal < 0)  // failed to connect
    stop = 1;    // set the flag so skip directly to tidy-up section

// ============== SEND REQUEST ======================================

  if (stop == 0)      // if we are connected
  {
    
    snprintf(request, MAXREQUEST, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", webDirectory, webAddress);

    printf("REQUEST: %s", request);
    

    /* Now send the request to the server over the TCP connection.
    send() arguments: socket identifier, array of bytes to send,
    number of bytes to send, and last argument of 0.  */
    reqLen = strlen(request);                         // get request length
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

  FILE *outFile;                        // Declare file pointer to output file
  outFile = fopen(fileSaveName, "wb");  // Open output file, in write binary mode. The file will be craeted if it didn't exist before, and overwritten if it did.

  /* Loop to receive the entire response - it could be long!  This
  loop ends when it is detected that we have received contentLength bytes after header,
  or when the server closes the connection, or when a problem occurs. */
  int endOfHeaderIdx = 0;               // When we find the header during the first iteration of the loop we set this as the index after the header so we can skip the header while saving the file
  while (stop == 0)                     // Stop will be set to 1 within the loop when we experience and error and want to exit the loop
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
        response[MAXRESPONSE] = '\0';                                   // make response a string so we can use string.h functions on it
        char *endmarker = strstr(response, "\r\n\r\n");                 // return pointer to first occurance of "\r\n\r\n" (endmarker) in the string or NULL otherwise
        char *findContentLenght = strstr(response, "Content-Length:");  // return pointer to first occurance of "Content-Length" in the string or NULL otherwise
        if (endmarker == NULL || findContentLenght == NULL)             // If either of these pointers are NULL either we did not receive a header or it is not as expected
        {
          printf("\n*** Did not receive an appropriate header\n");
          stop = 1;
          break;                                                        // We use break here to avoid writing anything and the numBytes read detection. (although harmless, who knows about future undefined behaviour
        }
        headerLength = (endmarker+4) - response;                        // Length of header in bytes is the index of the pointer to first \r in end marker + 4 - response pointer
        if (sscanf(findContentLenght+15, " %i\n", &contentLength) != 1) // If we don't read 1 integer into contentLength (sscanf returns number of successfull scans)
        {
          printf("\n*** Could not extract content-length from header\n");
          stop = 1;
          break;
        }
      }
      
      /*  We save all bytes into outFile. If this is our first iteration of the loop then numBytes == 0, and we start the index at headerLength in order to skip all bytes contained in the header */
      for (int i = (numBytes == 0 ? headerLength : 0); i<numRx; i++)
      {
        fwrite(&response[i], 1, sizeof(response[i]), outFile);          // Write response[i], 1 char, of size 1 byte, to file outFile
      }
      numBytes += numRx;    // add to byte counter
  
      // check if the client should close the connection early before attempting to receive bytes
      if (numBytes != 0 && numBytes - headerLength >= contentLength) // we have already received the amount of bytes indicated in the header 
      {
        printf("\nDetected that all bytes have been received.\n");
        break;                                                           // In this instance we don't set stop = 1 as it was successfull and stop will be our return value
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

  printf("\nClient is closing the connection...\n");

  // Close the socket and shut down the WSA system
  TCPcloseSocket(clientSocket);

  return stop;      // we only set stop = 1 in instances where the function failed to download the file, so return 1 = failure, while return 0 = success
} 

int main()
{

  // Print starting message
  printf("\nCommunication Systems client program\n\n");
  
  char yesOrNo[2];   // buffer to hold user input
  /* We chose a do-while loop as it would execute the block before checking uesr input to go again*/
  do
  {
    // Get the web address from the user
    // Some examples are: http://faraday1.ucd.ie/archive/thesis/masterthesis_federico_milano.pdf, or faraday1.ucd.ie or faraday1.ucd.ie/, or web.simmons.edu/~grovesd/images/big-bear.png
    printf("\nEnter the full web address you  would like to access (maximum %d bytes): ", MAXREQUEST-2); // prompt user to enter full web address
    read_line(fullWebAddress, MAXREQUEST, stdin);                    // read a maximum of MAXREQUEST chars from stdin into fullWebAddress
    printf("Please enter the name of the file to save as: ");        // prompt user to enter filename
    read_line(fileSaveName, 100, stdin);                             // read a maximum of 100 chars from stdin into fileSaveName

    if (downloadHttpFile())                                          // if downloadHttpFile() evaluates as false (returns stop = 0), it failed
    {
      printf("FAILURE, we're sorry we could not download your file. Please recheck your input and try again, or try again later."); // #saysorry
    }
    else  // yippee! The program believes it was successfull
    {
      printf("Your downloaded file has been saved to this directory as %s!", fileSaveName);
    }
    printf("To download another file enter 'y', otherwise enter any other character to end: "); // prompt the user to enter 'y' if they would like to go again
    read_line(yesOrNo, 2, stdin);                                                               // read a maximum of 2 chars (y\n) from stdin into yesOrNo
  }while (yesOrNo[0] == 'y');                                                                   // while yesOrNo[0] == 'y', we run everything inside the do loop.

  printf("\nSee you soon :)\n\n"); // End of program

  return 0;
}
