/****************************************************************************\
*
*	FlatAV Core Engine
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#ifndef FLATAV_H_SDIWU_
#define FLATAV_H_SDIWU_

#include <tchar.h>
#include "flatav-config.h"
#include "fvd.h"
#include "scanner.h"
/*#include "crc32.h"*/
/*#include "crc32file.h"*/

#ifdef __cplusplus
extern "C" {
#endif

#define LIBFLATAV_VERSION   "0.0.1"
#define LIBFLATAV_VERNUM    0x1

/* Return codes */
#define FL_CLEAN        0
#define FL_SUCCESS      0
#define FL_VIRUS        1   /* Found malware(s) */
#define FL_SUSPECT      2   /* File is suspected as malware */
#define FL_SUSPECTED    2

#define FL_ENOFILE      -101    /* No such file or directory */
#define FL_EIO          -102    /* Input/output error */
#define FL_EBADF        -103    /* Bad file descriptor */
#define FL_ENOMEM       -104    /* Not enough memory */
#define FL_EACCESS      -105    /* Permission denied */
#define FL_EMFILE       -106    /* Too many open files */
#define FL_EOPEN        -107    /* File open error */
#define FL_EWRITE       -108    /* File write error */
#define FL_ENULLARG     -109    /* NULL argument passed */
#define FL_ETEMPF       -110    /* Make temp file failed */
#define FL_EFVD         -111    /* Invalid fvd format*/
#define FL_EPE          -112    /* Invalid PE file */
#define FL_EARC         -113    /* Invalid archive file */
#define FL_ENOTSUPT     -114    /* Operation not supported */
#define FL_ENOSUPT      FL_ENOTSUPT

/* Fvd Functions */
LIBEXTERN int LIBEXPORT Fvd_BuildNewDb (const _TCHAR* szHDBFilePath, 
                                        const _TCHAR* szCDBFilePath, const _TCHAR* szXDBFilePath, 
                                        const _TCHAR* szSDBFilePath, const _TCHAR* szWDBFilePath,
                                        _TCHAR* szFVDFilePath);

LIBEXTERN int LIBEXPORT Fvd_LoadDb (_TCHAR* szDbFilePath, LPFVD_BLOCK lpFvdBlock);

LIBEXTERN void LIBEXPORT Fvd_UnloadDb (LPFVD_BLOCK lpFvdBlock);

LIBEXTERN void LIBEXPORT Fvd_ParseHeader (LPFVD_HEADER lpFvdHeader, 
                                          const LPFVD_BLOCK lpFvdBlock);

LIBEXTERN int LIBEXPORT Fvd_ValidateDb (const LPFVD_BLOCK lpFvdBlock, 
                                        unsigned short wCurMinAvMayorVersion, 
                                        unsigned char bCurMinAvMinorVersion);

LIBEXTERN void* LIBEXPORT Fvd_GetTblPtr (unsigned int dwTbl, int iFileType, 
                                         LPFVD_BLOCK lpFvdBlock);

LIBEXTERN unsigned int LIBEXPORT Fvd_GetTblRecCount (unsigned int dwTbl, 
                                                     int iFileType, 
                                                     LPFVD_BLOCK lpFvdBlock);

LIBEXTERN bool_t LIBEXPORT Fvd_LookupCWDB (LPFVD_FIND_CWDBREC lpFvdFindCwdbRec, 
                                           LPFVD_BLOCK lpFvdBlock);

/* Scanner Functions */
LIBEXTERN int LIBEXPORT Sca_ScanFile (const _TCHAR* szFilePathName, LPFVD_BLOCK lpFvdBlock);

#ifdef __cplusplus
}
#endif

#endif 