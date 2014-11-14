/****************************************************************************\
*
*   FILE:       macro.h
*
*   PURPOSE:    Contains useful macro.
*
*   MACROS:
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/
#ifndef MACRO_H_SDIWU_4540417
#define MACRO_H_SDIWU_4540417



/* Typecast macro for data type char* */
#define PTR(type, data) (*((type*)&(data)))
/* Typecast macro for data type void* */
#define PTROFF(type, data, offset) (*((type*)((unsigned char*)(data)+(offset))))
/* Align x to n bytes */
#define ALIGN(x, n) ((x + (n - 1)) & ~(n - 1))
#endif