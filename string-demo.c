/*  Program to demonstrate some string handling
    and conversion techniques in C.    */

#include <stdio.h>
#include <stdlib.h>  // needed for atoi, itoa, etc.
#include <string.h>  // needed for strlen, strtok

#define MAXFILENAME 50  // maximum length of file name
#define MAXPATHNAME 20  // maximum length of path
#define STRSIZE 50      // size of string

int main()
{
    char infoStr[STRSIZE];  // array to hold information as string
    char fName[MAXFILENAME+2];  // array to hold filename
    char data[MAXFILENAME+2];   // array to hold recovered string
    // string with path to files, with enough space to add file name
    // note double backslash in a string represents one backslash
    char pathWrite[MAXFILENAME+MAXPATHNAME+6] = "fileStore\\";
    char pathRead[MAXFILENAME+MAXPATHNAME+6]; // string for read path
    int retVal;         // return code from functions
    int len;            // length of a string
    int lenStr;         // length of another string
    int value = 12345;  // test value for conversion
    int num;            // number extracted from string
    long num2;          // another number extracted from a string
    char* ptr = NULL;   // pointer to position in string

//================ Get name of file from user ====================

    printf("\nEnter name of a file (name.ext): ");
    fgets(fName, MAXFILENAME, stdin);  // get filename

    /* fgets() will usually collect the end of line character as
    part of the string.  Need to find it and remove it.
    strcspn() gives the length of the string up to, but not including,
    any of the characters in the second string.   */
    len = strcspn(fName, "\n"); // find actual length of filename
    fName[len] = 0;     // mark this as end of string

//================ Build the file path ===========================

    strncpy(pathRead, pathWrite, MAXPATHNAME);  // copy the path string
    // this limits number of characters copied to MAXPATHNAME

    strncat(pathRead, fName, MAXFILENAME);  // add fName to end of path
    // for safety, strncat() limits the number of characters added
    printf("The full path to the file is %s\n", pathRead);

//================ Build another file path ===========================

    // This creates a filename with Z at the start...
    strncat(pathWrite, "Z ", 1);  // add one character to end of path
    strncat(pathWrite, fName, MAXFILENAME);  // add fName to this
    printf("The full path to the new file is %s\n", pathWrite);


// ================ Searching in a string =========================

    printf("\nSearching string |%s|\n", pathWrite);

    // Look for first dot character in the string
    ptr = strchr(pathWrite, '.');   // look for first dot
    if (ptr == NULL) printf("No dot found in string\n");
    else printf("String from dot onwards is |%s|\n", ptr);

    // Find position of dot or dash in string
    len = strcspn(pathWrite, ".-");
    if (len == strlen(pathWrite)) printf("No dot or dash found in string\n");
    else printf("%c found at position %d in string\n", pathWrite[len], len);

    // Find \Z in string
    ptr = strstr(pathWrite, "\\Z");  // look for sub-string in string
    if (ptr == NULL) printf("String \\Z not found\n");
    else printf("String from \\Z onwards is |%s|\n", ptr);


// ================ Convert number to string or vice-versa ========

    printf("\nThe value to be converted is %d\n", value);

    // Convert value to a string, in decimal
//    itoa(value, infoStr, 10);  // non-standard function, but common
    snprintf(infoStr, STRSIZE, "%d", value);  // use this if itoa is not available
	lenStr = strlen(infoStr);  // get length of this string
    printf("Value as string: %s, length %d\n", infoStr, lenStr);

    // Convert string back to number
    num = atoi(infoStr);
    printf("Number extracted from string = %d\n", num);


// ================ Put multiple items in a string ================

    // Use snprintf() to write filename and size to a string
    // This example uses * characters to separate items
    lenStr = snprintf(infoStr, STRSIZE, "Xyz*%s*%d", fName, value);
    printf("\nGenerated string: |%s|, reported length %d\n", infoStr, lenStr);

    /* If there are more than STRSIZE-1 characters to be written to
    the string, snprintf will return the number to be written, but
    only write STRSIZE-1 (plus the end of string marker). */
    lenStr = strlen(infoStr);   // measure the length of the string
    printf("Measured length of string: %d\n", lenStr);

    // sizeof() gives the amount of storage allocated to the array, in bytes.
    // This is not normally useful...
    printf("Size of string array: %ld\n", sizeof(infoStr));


// ================ Extract items from a string using strtok() ================

    /* strtok() is designed to break a string into separate parts, or tokens,
    based on some defined set of delimiters or marker characters.
    In this example, the * character is the only delimiter or marker - to
    suit the string created above.  The first item in the string is all the
    characters up to, but not including, the first *.  The second part is
    the characters between the two *s.  This can continue for many parts. */

    char *strPtr;  // pointer to a string fragment
    lenStr = strlen(infoStr);      // save the length of the full string
    strPtr = strtok(infoStr, "*");  // get first item from string
    len = strlen(strPtr);  // get the length of this item
	if(len == lenStr)	// lengths match - the first token is the entire string
		printf("\nNo delimiter found in string.");
    printf("\nExtracted  first part: %s, length %d\n", strPtr, len);

    // After the first call, use NULL to continue processing the same string
    strPtr = strtok(NULL, "*");  // get second item from string
    if (strPtr == NULL)
    {
        printf("Already at end of string\n");
        return 0;
    }
    len = strlen(strPtr);  // get length of this
    printf("Extracted second part: %s, length %d\n", strPtr, len);

    strPtr = strtok(NULL, "*");  // get third item from string
    if (strPtr == NULL)
    {
        printf("Already at end of string\n");
        return 0;
    }
    len = strlen(strPtr);  // get length of this
    printf("Extracted  third part: %s, length %d\n", strPtr, len);

    num2 = strtol(strPtr, NULL, 10);  // convert to integer, using base 10
    printf("Third part as a number: %ld\n", num2);


// ================ Alternative, using sscanf() ================

    // Write filename and size to a string, separated by spaces
    lenStr = snprintf(infoStr, STRSIZE, "%s %d", fName, value);
    printf("\nGenerated string: |%s|, length %d\n", infoStr, lenStr);

    /* Use sscanf() to extract the data.  This works like scanf(),
    but reads from a string instead of input.  The return value is
    the number of items extracted from the string.  This method will
    fail if one of the items to be extracted includes a space.
	There are more advanced ways of using sscanf...  */
    retVal = sscanf(infoStr, "%s %d", data, &num);
    if (retVal == 2) // extracted 2 items as required
    {
        printf("Extracted filename: %s\n", data);
        printf("Extracted value: %d\n", num);
    }
    else
        printf("Problem extracting values, sscanf() returned %d\n", retVal);

    return 0;
}
