/*  Program to demonstrate file handling and some string
    handling techniques in C.
    Includes: building a file path;
			file open for binary read (stream of bytes);
            finding file size; changing position in file;
			reading blocks of bytes from a file;
			file open for binary write;
			writing blocks of bytes to a file;
			closing files						*/

#include <stdio.h>
#include <errno.h>   // contains errno = error number
#include <string.h>  // needed for strncat, strcpy

#define BLOCKSIZE 40    // number of bytes to read at a time
#define MAXFILENAME 70  // maximum length of file name
#define MAXPATHNAME 20  // maximum length of path
#define BYTESPERLINE 10 // number of bytes to print on each line
#define MINTEXT 10      // limits for text characters
#define MAXTEXT 127

int main()
{
    char fName[MAXFILENAME+2];  // array to hold filename
    char data[BLOCKSIZE+2];     // array to hold file data
    /* string with path to files, with enough space to add the file name.
       Note double backslash in a string represents one backslash.
       The backslash is used to separate items in a path in Windows.
       For Linux, use a forward slash. */
    char pathWrite[MAXFILENAME+MAXPATHNAME] = "fileStore/";  // Linux
    // char pathWrite[MAXFILENAME+MAXPATHNAME] = "fileStore\\";  // Windows
    char pathRead[MAXFILENAME+MAXPATHNAME]; // string for read path
    FILE *fp;          // file handle for file
    int retVal, rv2;   // return codes from functions
    int i;             // used in for loop
    unsigned char bVal;    // byte value
    int minB=255, maxB=0;  // min and max byte values
    long nBytes;       // file size
    long totalBytes = 0;    // total bytes read from file
    size_t len;        // length of a string

//================ Get name of file from user ====================

    printf("\nEnter name of file (name.ext): ");
    fgets(fName, MAXFILENAME, stdin);  // get filename

    /* fgets() will usually collect the end of line character as
    part of the string.  Need to find it and remove it.
    strcspn() give the length of the string up to, but not including,
    any of the characters in the second string.   */
    len = strcspn(fName, "\n"); // find actual length of filename
    fName[len] = 0;     // mark this as end of string

//================ Build the file path ===========================

    strncpy(pathRead, pathWrite, MAXPATHNAME);  // copy the path string
    // this limits number of characters copied to MAXPATHNAME

    strncat(pathRead, fName, MAXFILENAME);  // add fName to end of path
    // for safety, strncat() limits the number of characters added

//================ Open the file for reading, as bytes ===========

    printf("\nOpening %s for binary read\n", pathRead);
    fp = fopen(pathRead, "rb");  // open for binary read
    if (fp == NULL)     // check for problem
    {
        perror("Problem opening file");  // perror() gives details
        printf("errno = %d\n", errno); // errno allows program to react
        return 1;
    }

// ================ Find the size of the open file ================

    // First move to the end of the file
    retVal = fseek(fp, 0, SEEK_END);  // set position to end of file
    if (retVal != 0)  // if there was a problem
    {
        perror("Problem in fseek");
        printf("errno = %d\n", errno);
        fclose (fp);
        return 2;
    }

    // Then find the position, measured in bytes from start of file
    nBytes = ftell(fp);         // find current position
    printf("\nFile size is %ld bytes\n", nBytes);  // print it

    // To read the file, usually want to start at the beginning again
    retVal = fseek(fp, 0, SEEK_SET);   // set position to start of file
    if (retVal != 0)  // if there was a problem
    {
        perror("Problem in fseek");
        printf("errno = %d\n", errno);
        fclose (fp);
        return 3;
    }

// ================ Read bytes from the open file ================

    // Read the first block of data and print it (just for demonstration)
    retVal = (int) fread(data, 1, BLOCKSIZE, fp);  // read block of bytes
    if (ferror(fp))  // check for problem
    {
        perror("Problem reading input file");
        printf("errno = %d\n", errno);
        fclose(fp);
        return 4;
    }
    if (retVal > 0 )  // we got some bytes - print them
    {
        printf("\nRead first %d bytes: \n", retVal);
        totalBytes += retVal;       // add to byte counter
        for(i=0; i<retVal; i++)     // step through the bytes
        {
            bVal = (unsigned char) data[i]; // get a byte value
            printf("%d ", bVal);  // print one byte at a time, as number
            if (i%BYTESPERLINE == BYTESPERLINE-1) printf("\n");  // new line
            if (bVal < minB) minB = bVal;  // update minB
            if (bVal > maxB) maxB = bVal;  // update maxB
        }

        // if it seems to be text, print it as a string
        if ((minB >= MINTEXT) && (maxB <= MAXTEXT))
        {
            data[retVal] = 0;  // add null marker to make a string
            printf("\nAs string:\n%s\n", data);
        }
        else printf("\n");  // otherwise just tidy up
    }
    else    // fread did not get any bytes
        printf("\nNo data in file\n");

// ================ Read the rest of the file ================

    while (!feof(fp))   // continue until end of file
    {
        retVal = (int) fread(data, 1, BLOCKSIZE, fp);  // read bytes
        if (ferror(fp))  // check for problem
        {
            perror("Problem reading input file");
            printf("errno = %d\n", errno);
            fclose(fp);
            return 4;
        }
        totalBytes += retVal;   // add to byte counter
        printf("Read %d bytes from the file\n", retVal);

    /* You would normally do something useful with the block of data at
       this point, but in this demonstration, there is nothing to do. */

    }

    printf("End of file after %ld bytes\n", totalBytes);


// ================ Close the open file ================

    fclose (fp);        // close the file
    printf("\nInput file closed\n");

// ===============================================================
// ================ Example writing to a file  ===================

//================ Build the file path ===========================

     strncat(pathWrite, "Z ", 1);  // add one character to end of path
     strncat(pathWrite, fName, MAXFILENAME);  // add fName to this


//================ Open the file for writing, as bytes ===========

    printf("\nOpening %s for binary write\n", pathWrite);
    fp = fopen(pathWrite, "wb");  // open for binary write
    // this method will over-write any existing file of that name
    if (fp == NULL) // check for problem
    {
        perror("Problem opening file");
        printf("errno = %d\n", errno);
        return 1;
    }

// ================ Write bytes to the open file ================

    // Write one block of data - just the last block from above
    rv2 = (int) fwrite(data, 1, retVal, fp);  // write bytes
    if (ferror(fp))  // check for problem
    {
        perror("Problem writing input file");
        printf("errno = %d\n", errno);
        fclose(fp);
        return 4;
    }
    printf("Wrote %d bytes to output file\n", rv2);

// ================ Close the open file ================

    fclose (fp);        // close the file
    printf("\nOuptut file closed\n");

    return 0;
}
