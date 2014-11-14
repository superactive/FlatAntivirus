/* unzip.h -- IO for uncompress .zip files using zlib
   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant

   This unzip package allow extract file from .ZIP file, compatible with PKZip 2.04g
     WinZip, InfoZip tools and compatible.

   Multi volume ZipFile (span) are not supported.
   Encryption compatible with pkzip 2.04g only supported
   Old compressions used by old PKZip 1.x are not supported


   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

   Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.


*/

/* for more info about .ZIP format, see
      http://www.info-zip.org/pub/infozip/doc/appnote-981119-iz.zip
      http://www.info-zip.org/pub/infozip/doc/
   PkWare has also a specification at :
      ftp://ftp.pkware.com/probdesc.zip
*/

#ifndef _unz_H
#define _unz_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZLIB_H
#include "zlib.h"
#endif

#ifndef _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#include <tchar.h>

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagunzFile__ { int unused; } unzFile__;
typedef unzFile__ *unzFile;
#else
typedef voidp unzFile;
#endif

/* Return codes */
#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

/* tm_unz contain date/time info */
typedef struct tm_unz_s
{
    uInt tm_sec;            /* seconds after the minute - [0,59] */
    uInt tm_min;            /* minutes after the hour - [0,59] */
    uInt tm_hour;           /* hours since midnight - [0,23] */
    uInt tm_mday;           /* day of the month - [1,31] */
    uInt tm_mon;            /* months since January - [0,11] */
    uInt tm_year;           /* years - [1980..2044] */
} tm_unz;

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
typedef struct unz_global_info_s
{
    uLong number_entry;         /* total number of entries in the central dir on this disk */
    uLong size_comment;         /* size of the global comment of the zipfile */
} unz_global_info;


/* unz_file_info contain information about a file in the zipfile */
typedef struct unz_file_info_s
{
    uLong version;              /* version made by                 2 bytes */
    uLong version_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_method;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong crc;                  /* crc-32                          4 bytes */
    uLong compressed_size;      /* compressed size                 4 bytes */
    uLong uncompressed_size;    /* uncompressed size               4 bytes */
    uLong size_filename;        /* filename length                 2 bytes */
    uLong size_file_extra;      /* extra field length              2 bytes */
    uLong size_file_comment;    /* file comment length             2 bytes */

    uLong disk_num_start;       /* disk number start               2 bytes */
    uLong internal_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    tm_unz tmu_date;
} unz_file_info;

extern int ZEXPORT unzStringFileNameCompare OF ((const char* fileName1,
                                                 const char* fileName2,
                                                 int iCaseSensitivity));
/*
    PURPOSE
        Compare two filename (fileName1,fileName2).
    PARAMS
        fileName1
            First filename to be compared.
        fileName2
            Second filename to be compared.
        iCaseSensitivity
            If iCaseSenisivity = 0, case sensitivity is default of your operating system (like 1 on Unix, 2 on Windows)
            If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
            If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi or strcasecmp)    
    RETURNS
        Same as strcmp/strcmpi.
*/


extern unzFile ZEXPORT unzOpen OF((const _TCHAR *path));
/*
    PURPOSE
        Open a zip file for unzipping. 
    PARAMS
        path 
            Contains the full pathname (by example, on a Windows XP computer 
            "c:\\zlib\\zlib113.zip" or on an Unix computer "zlib/zlib113.zip".
    RETURNS
        Handle of unzip file.
    REMARKS
        If the zipfile cannot be opened (file don't exist or in not valid), the 
        return  value is NULL.
        Else, the return value is a unzFile Handle, usable with other function
        of this unzip package.
*/

extern unzFile ZEXPORT unzOpen2 OF((const _TCHAR *path,
                                    zlib_filefunc_def* pzlib_filefunc_def));
/*
/*
    PURPOSE
        Open a zip file for unzipping. 
    PARAMS
        path 
            Contains the full pathname (by example, on a Windows XP computer 
            "c:\\zlib\\zlib113.zip" or on an Unix computer "zlib/zlib113.zip".
        pzlib_filefunc_def
            A set of file low level API for read/write the zip file (see ioapi.h).
    RETURNS
        Handle of unzip file.
    REMARKS
        If the zipfile cannot be opened (file don't exist or in not valid), the 
        return  value is NULL.
        Else, the return value is a unzFile Handle, usable with other function
        of this unzip package.
*/

extern int ZEXPORT unzClose OF((unzFile unzfile));
/*
    PURPOSE
        Close a ZipFile opened with unzipOpen.
    PARAMS
        unzfile
            Handle of opened unzip file.
    RETURNS
        UNZ_OK if there is no error.
    REMARKS
        If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
        these files MUST be closed with unzCloseCurrentFile before call unzClose.
*/

extern int ZEXPORT unzGetGlobalInfo OF((unzFile unzfile,
                                        unz_global_info *pglobal_info));
/*
    PURPOSE
        Get global info about the ZipFile.
    PARAMS
        unzfile
            Handle of opened unzip file.
        pglobal_info
            Pointer to global info structure to receive the global info.
    RETURNS
        UNZ_OK if there is no error.
*/


extern int ZEXPORT unzGetGlobalComment OF((unzFile unzfile,
                                           char *szComment,
                                           uLong uSizeBuf));
/*
    PURPOSE
        Get the global comment string of the ZipFile.
    PARAMS
        unzfile
            Handle of opened unzip file.
        szComment
            Pointer to string buffer to receive the comment.
        uSizeBuf
            Specifies the size of string buffer szComment
    RETURNS
        The number of byte copied or an error code < 0.
*/

/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

extern int ZEXPORT unzGoToFirstFile OF((unzFile unzfile));
/*
    PURPOSE
        Set the current file of the zipfile to the first file.
    PARAMS
        unzfile
            Handle of opened unzip file.
    RETURNS
        UNZ_OK if there is no error.
*/

extern int ZEXPORT unzGoToNextFile OF((unzFile unzfile));
/*
    PURPOSE
        Set the current file of the zipfile to the next file.
    PARAMS
        unzfile
            Handle of opened unzip file.
    RETURNS
        UNZ_OK if there is no error.
        UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

extern int ZEXPORT unzLocateFile OF((unzFile unzfile,
                                     const char *szFileName,
                                     int iCaseSensitivity));
/*
    PURPOSE
        Try locate the file szFileName in the zipfile.
    PARAMS
        unzfile
            Handle of opened unzip file.
        szFileName
            File name to be located in unzip file.
        iCaseSensitivity
            Case sensitivity flag. See unzStringFileNameCompare.
    RETURNS
        UNZ_OK if the file is found. It becomes the current file.
        UNZ_END_OF_LIST_OF_FILE if the file is not found.
*/

/* ****************************************** */
/* Ryan supplied functions */
/* unz_file_info contain information about a file in the zipfile */

typedef struct unz_file_pos_s
{
    uLong pos_in_zip_directory;   /* offset in zip file directory */
    uLong num_of_file;            /* # of file */
} unz_file_pos;

extern int ZEXPORT unzGetFilePos(unzFile unzfile,
                                 unz_file_pos* file_pos);
/*
    PURPOSE
        Get current file position in the zipfile.
    PARAMS
        unzfile
            Handle of opened unzip file.
        file_pos
            Pointer to unz_file_pos structure to receive the value.
    RETURNS
        UNZ_OK if the file is found.
        UNZ_END_OF_LIST_OF_FILE if the file is not found.
        UNZ_PARAMERROR if the parameter is invalid.
*/

extern int ZEXPORT unzGoToFilePos(unzFile unzfile,
                                  unz_file_pos* file_pos);
/*
    PURPOSE
        Set file position in the zipfile.
    PARAMS
        unzfile
            Handle of opened unzip file.
        file_pos
            Pointer to unz_file_pos structure to pass the information.
    RETURNS
        UNZ_OK if the file is found.
        UNZ_PARAMERROR if the parameter is invalid.
*/

/* ****************************************** */

extern int ZEXPORT unzGetCurrentFileInfo OF((unzFile unzfile,
                                             unz_file_info *pfile_info,
                                             char *szFileName,
                                             uLong fileNameBufferSize,
                                             void *extraField,
                                             uLong extraFieldBufferSize,
                                             char *szComment,
                                             uLong commentBufferSize));
/*
    PURPOSE
        Get info about the current file inside unzip file.
    PARAM
        unzfile
            Handle of opened unzip file.
        pfile_info
            Pointer to unz_file_info structure to receive current file info.
            It can be NULL, if the data is not required.
        szFileName
            Pointer to string buffer to receive current filename.
            It can be NULL, if the data is not required.
        fileNameBufferSize
            Size of string buffer szFileName.
        extraField
            Pointer to buffer to receive extra field information.
            This is the Central-header version of the extra field.
            It can be NULL, if the data is not required.
        extraFieldBufferSize
            Size of buffer extraField.
        szComment
            Pointer to buffer to receive comment.
            It can be NULL, if the data is not required.
        commentBufferSize
            Size of buffer szComment.
    RETURNS
        UNZ_OK if there is no error.
*/

/***************************************************************************/
/* 
    for reading the content of the current zipfile, you can open it, read data
    from it, and close it (you can close it before reading all the file)
*/

extern int ZEXPORT unzOpenCurrentFile OF((unzFile unzfile));
/*
    PURPOSE
        Open unzip file for reading data.
    PARAM
        unzfile
            Handle of opened unzip file.
    RETURNS
        UNZ_OK if there is no error.
*/

extern int ZEXPORT unzOpenCurrentFilePassword OF((unzFile unzfile,
                                                  const char* password));
/*
    PURPOSE
        Open unzip file for reading data.
    PARAM
        unzfile
            Handle of opened unzip file.
        password
            Password for crypted zip file.
    RETURNS
        UNZ_OK if there is no error.
*/

extern int ZEXPORT unzOpenCurrentFile2 OF((unzFile unzfile,
                                           int* method,
                                           int* level,
                                           int raw));
/*
    PURPOSE
        Open unzip file for reading data.
    PARAM
        unzfile
            Handle of opened unzip file.
        method
            Pointer to buffer to receive the method of compression.
        level
            Pointer to buffer to receive the level of compression.
            It can be NULL, if the data is not required.
        raw
            if 1, open for reading raw data (no uncompress).
    RETURNS
        UNZ_OK if there is no error.
*/

extern int ZEXPORT unzOpenCurrentFile3 OF((unzFile unzfile,
                                           int* method,
                                           int* level,
                                           int raw,
                                           const char* password));
/*
    PURPOSE
        Open unzip file for reading data.
    PARAM
        unzfile
            Handle of opened unzip file.
        method
            Pointer to buffer to receive the method of compression.
        level
            Pointer to buffer to receive the level of compression.
            It can be NULL, if the data is not required.
        raw
            if 1, open for reading raw data (no uncompress).
        password
            Password for crypted zip file.
    RETURNS
        UNZ_OK if there is no error.
*/


extern int ZEXPORT unzCloseCurrentFile OF((unzFile unzfile));
/*
    PURPOSE
        Close the file in zip opened with unzOpenCurrentFile.
    PARAM
        unzfile
            Handle of opened unzip file.
    RETURNS
        UNZ_OK if there is no error.
        UNZ_CRCERROR if all the file was read but the CRC is not good.
*/

extern int ZEXPORT unzReadCurrentFile OF((unzFile unzfile,
                                          voidp buf,
                                          unsigned len));
/*
    PURPOSE
        Read bytes from the current file inside zip file opened by unzOpenCurrentFile.
    PARAM
        unzfile
            Handle of opened unzip file.
        buf
            Point to buffer to receive bytes of current file in zip.
        len
            Size of buffer.
    RETURNS
        The number of byte copied if somes bytes are copied.
        0 if the end of file was reached.
        < 0 with error code if there is an error (UNZ_ERRNO for IO error, 
            or zLib error for uncompress error).
*/


extern z_off_t ZEXPORT unztell OF((unzFile unzfile));
/*
    PURPOSE
        Give the current position in uncompressed data.
    PARAM
        unzfile
            Handle of opened unzip file.
    RETURNS
        Current file position in uncompressed data.
*/

extern int ZEXPORT unzeof OF((unzFile unzfile));
/*
    PURPOSE
        Check for EOF.
    PARAM
        unzfile
            Handle of opened unzip file.
    RETURNS
        1 if the end of file was reached or 0 for the else.
*/

extern int ZEXPORT unzGetLocalExtrafield OF((unzFile unzfile,
                                             voidp buf,
                                             unsigned len));
/*
    PURPOSE
        Read extra field from the current file (opened by unzOpenCurrentFile).
        This is the local-header version of the extra field (sometimes, there is 
        more info in the local-header version than in the central-header).
    PARAMS
        unzfile
            Handle of opened unzip file.
        buf
            Pointer to buffer to receive local extra field information.
            If buf is NULL, function will returns the size of the local extra field.
        len
            If buf is not NULL, len contains the size of the buf.
    RETURNS
        If buf is not NULL, returns the number of bytes copied in buf, or error code.
        If buf is NULL, returns the size of local extra field.
*/

/***************************************************************************/

/* Get the current file offset */
extern uLong ZEXPORT unzGetOffset (unzFile unzfile);

/* Set the current file offset */
extern int ZEXPORT unzSetOffset (unzFile unzfile, uLong pos);


#ifdef __cplusplus
}
#endif

#endif /* _unz_H */
