/****************************************************************************\
*
*	Script File Processing Functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "macro.h"

#define UNICODE_LE_BYTE_MARK    0xfeff	// Unicode little endian (intel x86)
#define UNICODE_BE_BYTE_MARK    0xfffe	// Unicode big endian
#define UTF8_BYTE_MARK          0x00bfbbef

static void SwapBytes (char* buffer, int len) {
    int i;
	for (i = 0; i < len; i += 2) {
		char b = buffer[i];
		buffer[i] = buffer[i+1];
		buffer[i+1] = b;
	}
}

//int Script_Normalize2 (char* buffer, unsigned int len)
//{
//    unsigned int i, j;
//    char* tempBuff;
//
//    tempBuff = (char*) malloc (len);
//    if (tempBuff == NULL)
//        return -1;
//
//    if (PTR(unsigned short, buffer[0]) == UNICODE_LE_BYTE_MARK) {
//        for (i = 0, j = 0; i < len; i+=2) {
//            if (PTR(unsigned short, buffer[i]) != 0x20 && 
//                PTR(unsigned short, buffer[i]) != 0xd &&
//                PTR(unsigned short, buffer[i]) != 0xa) 
//            {
//                tempBuff[j] = buffer[i];
//                tempBuff[j+1] = buffer[i+1];
//                j += 2;
//            }
//        }
//    }
//    else {
//        for (i = 0, j = 0; i < len; ++i) {
//            if (buffer[i] != 0x20 && 
//                buffer[i] != 0xd &&
//                buffer[i] != 0xa)
//            {
//                tempBuff[j] = buffer[i];
//                ++j;
//            }
//        }
//    }
//
//    memset (buffer, '\0', len);
//    memcpy (buffer, tempBuff, j);
//    free (tempBuff);
//    return 0;
//}

/*
	Strip out some whitespace characters (linefeed, tab, space) from the script contents.
*/
int Script_Normalize (char* buffer, unsigned int len)
{
    unsigned int i, j;
    char* normalBuff;

    normalBuff = (char*) malloc (len);
    if (normalBuff == NULL)
        return -1;

    /* If file type is unicode big endian, convert it little endian by swaping its bytes */
    if (PTR(unsigned short, buffer[0]) == UNICODE_BE_BYTE_MARK)
        SwapBytes (buffer, len);

    if (PTR(unsigned short, buffer[0]) == UNICODE_LE_BYTE_MARK) {
        char* ansiBuff;
        ansiBuff = (char*) malloc (len);
        if (ansiBuff == NULL)
            return -1;
        /* Convert wide-char string to ansi-char string */
        wcstombs (ansiBuff, (wchar_t*) &buffer[2], len);
        for (i = 0, j = 0; i < len; ++i) {
            if (ansiBuff[i] != 0x20 && 
                ansiBuff[i] != 0xd &&
                ansiBuff[i] != 0xa &&
                ansiBuff[i] != 9)
            {
                normalBuff[j] = ansiBuff[i];
                ++j;
            }
        }

        free (ansiBuff);
    }
    else {
        for (i = 0, j = 0; i < len; ++i) {
            if (buffer[i] != 0x20 && 
                buffer[i] != 0xd &&
                buffer[i] != 0xa &&
                buffer[i] != 9)
            {
                normalBuff[j] = buffer[i];
                ++j;
            }
        }
    }

    memset (buffer, '\0', len);
    memcpy (buffer, normalBuff, j);
    free (normalBuff);
    return 0;
}
