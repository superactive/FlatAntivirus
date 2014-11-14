/* zip.h -- IO for compress .zip files using zlib
   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant

   This unzip package allow creates .ZIP file, compatible with PKZip 2.04g
     WinZip, InfoZip tools and compatible.
   Multi volume ZipFile (span) are not supported.
   Encryption compatible with pkzip 2.04g only supported
   Old compressions used by old PKZip 1.x are not supported

  For uncompress .zip file, look at unzip.h


   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.html for evolution

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

#ifndef _zip_H
#define _zip_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZLIB_H
#include "zlib.h"
#endif

#ifndef _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#if defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagzipFile__ { int unused; } zipFile__;
typedef zipFile__ *zipFile;
#else
typedef voidp zipFile;
#endif

/* Return codes */
#define ZIP_OK                          (0)
#define ZIP_EOF                         (0)
#define ZIP_ERRNO                       (Z_ERRNO)
#define ZIP_PARAMERROR                  (-102)
#define ZIP_BADZIPFILE                  (-103)
#define ZIP_INTERNALERROR               (-104)

#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif
/* default memLevel */

/* tm_zip contain date/time info */
typedef struct tm_zip_s
{
    uInt tm_sec;            /* seconds after the minute - [0,59] */
    uInt tm_min;            /* minutes after the hour - [0,59] */
    uInt tm_hour;           /* hours since midnight - [0,23] */
    uInt tm_mday;           /* day of the month - [1,31] */
    uInt tm_mon;            /* months since January - [0,11] */
    uInt tm_year;           /* years - [1980..2044] */
} tm_zip;

typedef struct
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;        /* if dos_date == 0, tmu_date is used      */
/*  uLong       flag;              general purpose bit flag        2 bytes */

    uLong       internal_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

typedef const char* zipcharpc;

/* append flag */
#define APPEND_STATUS_CREATE        (0) /* Create new zip file */
#define APPEND_STATUS_CREATEAFTER   (1) /* Create new zip file in the end of file */
#define APPEND_STATUS_ADDINZIP      (2) /* Add files to a zip file */

extern zipFile ZEXPORT zipOpen OF((const _TCHAR *pathname, int append));
/*
    PURPOSE
        Create a zipfile.
    PARAMS
        pathname
            Contains zip file pathname on Windows XP a filename like "c:\\zlib\\zlib113.zip" or on
            an Unix computer "zlib/zlib113.zip".
        append
            Contains append flag, see above.
    RETURNS
        zip file handle or NULL if failed.
    REMARK
        If the append flag is APPEND_STATUS_CREATE, new zip file will be created.
        If the append flag is APPEND_STATUS_CREATEAFTER, the zip will be created at 
        the end of the file (useful if the file contain a self extractor code).
        If the append flag is APPEND_STATUS_ADDINZIP, we will
        add files in existing zip (be sure you don't add file that doesn't exist).
*/

/* 
    Note : there is no delete function into a zipfile.
    If you want delete file into a zipfile, you must open a zipfile, and create another 
    zip file.
    Of course, you can use RAW reading and writing to copy the file you did not want to 
    delete.
*/

extern zipFile ZEXPORT zipOpen2 OF((const _TCHAR *pathname,
                                    int append,
                                    zipcharpc* globalcomment,
                                    zlib_filefunc_def* pzlib_filefunc_def));
/*
    PURPOSE
        Create a zipfile.
    PARAMS
        pathname
            Contains zip file pathname on Windows XP a filename like "c:\\zlib\\zlib113.zip" or on
            an Unix computer "zlib/zlib113.zip".
        append
            Contains append flag, see above.
        globalcomment
            Global comment to be passed.
        pzlib_filefunc_def
            A set of file low level API for read/write the zip file (see ioapi.h).
    RETURNS
        zip file handle or NULL if failed.
    REMARK
        If the append flag is APPEND_STATUS_CREATE, new zip file will be created.
        If the append flag is APPEND_STATUS_CREATEAFTER, the zip will be created at 
        the end of the file (useful if the file contain a self extractor code).
        If the append flag is APPEND_STATUS_ADDINZIP, we will
        add files in existing zip (be sure you don't add file that doesn't exist).
*/

extern int ZEXPORT zipOpenNewFileInZip OF((zipFile zipfile,
                                           const char* filename,
                                           const zip_fileinfo* zipfi,
                                           const void* extrafield_local,
                                           uInt size_extrafield_local,
                                           const void* extrafield_global,
                                           uInt size_extrafield_global,
                                           const char* comment,
                                           int method,
                                           int level));
/*
    PURPOSE
        Open a file in the ZIP for writing.
    PARAMS
        zipfile
            Handle of opened zipfile.
        filename
            The filename in zip (if NULL, '-' without quote will be used).
        *zipfi 
            Zip fileinfo, contains supplemental information.
        extrafield_local
            If extrafield_local!=NULL and size_extrafield_local>0, extrafield_local
            contains the extrafield data the the local header.
        size_extrafield_local
            Size of extrafield_local.
        extrafield_global
            If extrafield_global!=NULL and size_extrafield_global>0, extrafield_global
            contains the extrafield data the the local header.
        size_extrafield_global
            Size of extrafield_global.
        comment
            If comment != NULL, comment contain the comment string.
        method
            Contains the compression method (0 for store, Z_DEFLATED for deflate).
        level
            Contains the level of compression (can be Z_DEFAULT_COMPRESSION).
    RETURNS
        ZIP_OK if no error.
*/


extern int ZEXPORT zipOpenNewFileInZip2 OF((zipFile zipfile,
                                            const char* filename,
                                            const zip_fileinfo* zipfi,
                                            const void* extrafield_local,
                                            uInt size_extrafield_local,
                                            const void* extrafield_global,
                                            uInt size_extrafield_global,
                                            const char* comment,
                                            int method,
                                            int level,
                                            int raw));

/*
    PURPOSE
        Open a file in the ZIP for writing.
    PARAMS
        zipfile
            Handle of opened zipfile.
        filename
            The filename in zip (if NULL, '-' without quote will be used).
        *zipfi 
            Zip fileinfo, contains supplemental information.
        extrafield_local
            If extrafield_local!=NULL and size_extrafield_local>0, extrafield_local
            contains the extrafield data the the local header.
        size_extrafield_local
            Size of extrafield_local.
        extrafield_global
            If extrafield_global!=NULL and size_extrafield_global>0, extrafield_global
            contains the extrafield data the the local header.
        size_extrafield_global
            Size of extrafield_global.
        comment
            If comment != NULL, comment contain the comment string.
        method
            Contains the compression method (0 for store, Z_DEFLATED for deflate).
        level
            Contains the level of compression (can be Z_DEFAULT_COMPRESSION).
        raw
            If 1, we write raw file.
    RETURNS
        ZIP_OK if no error.
 */

extern int ZEXPORT zipOpenNewFileInZip3 OF((zipFile zipfile,
                                            const char* filename,
                                            const zip_fileinfo* zipfi,
                                            const void* extrafield_local,
                                            uInt size_extrafield_local,
                                            const void* extrafield_global,
                                            uInt size_extrafield_global,
                                            const char* comment,
                                            int method,
                                            int level,
                                            int raw,
                                            int windowBits,
                                            int memLevel,
                                            int strategy,
                                            const char* password,
                                            uLong crcForCtypting));

/*
    PURPOSE
        Open a file in the ZIP for writing.
    PARAMS
        zipfile
            Handle of opened zipfile.
        filename
            The filename in zip (if NULL, '-' without quote will be used).
        *zipfi 
            Zip fileinfo, contains supplemental information.
        extrafield_local
            If extrafield_local!=NULL and size_extrafield_local>0, extrafield_local
            contains the extrafield data the the local header.
        size_extrafield_local
            Size of extrafield_local.
        extrafield_global
            If extrafield_global!=NULL and size_extrafield_global>0, extrafield_global
            contains the extrafield data the the local header.
        size_extrafield_global
            Size of extrafield_global.
        comment
            If comment != NULL, comment contain the comment string.
        method
            Contains the compression method (0 for store, Z_DEFLATED for deflate).
        level
            Contains the level of compression (can be Z_DEFAULT_COMPRESSION).
        raw
            If 1, we write raw file.
        windowBits
            The windowBits parameter is the base two logarithm of the window size.
            Default value is -MAX_WBITS.
        memLevel
            The memLevel parameter specifies how much memory should be allocated 
            for the internal compression state. Default value is DEF_MEM_LEVEL.
        strategy
            The strategy parameter is used to tune the compression algorithm.
            Default value is Z_DEFAULT_STRATEGY.
        password
            Password for crypting or NULL for no crypting.
        crcForCtypting
            CRC32 value of file to be compressed (needed for crypting) or 0 if no crypting.
    RETURNS
        ZIP_OK if no error.
 */


extern int ZEXPORT zipWriteInFileInZip OF((zipFile zipfile,
                                           const void* buf,
                                           unsigned len));
/*
    PURPOSE
        Write data in the zipfile
    PARAMS
        zipfile
            Handle of opened zip file.
        buf
            Buffer to be written to zip file.
        len
            Length of buffer.
    RETURNS
        ZIP_OK if no error.
*/

extern int ZEXPORT zipCloseFileInZip OF((zipFile zipfile));
/*
    PURPOSE
        Close the current file in the zipfile
    PARAMS
        zipfile
            Handle of opened zip file.
    RETURNS
        ZIP_OK if no error.
*/

extern int ZEXPORT zipCloseFileInZipRaw OF((zipFile zipfile,
                                            uLong uncompressed_size,
                                            uLong crc32));
/*
    PURPOSE
        Close the current file in the zipfile, for file opened with parameter raw = 1 
        in zipOpenNewFileInZip2.
    PARAMS
        zipfile
            Handle of opened zip file.
        uncompressed_size
            Uncompressed size of file.
        crc32
            CRC32 value of uncompressed file.
    RETURNS
        ZIP_OK if no error.
*/

extern int ZEXPORT zipClose OF((zipFile zipfile,
                                const char* global_comment));
/*
    PURPOSE
        Close the zipfile
    PARAMS
        zipfile
            Handle of opened zip file.
        global_comment
            Global comment to be added.
    RETURNS
        ZIP_OK if no error.
*/

#ifdef __cplusplus
}
#endif

#endif /* _zip_H */
