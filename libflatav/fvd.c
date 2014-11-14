/****************************************************************************\
*
*   FILE:       fvd.c
*
*   PURPOSE:    Functions for manipulating FlatAV database.
*
*   FUNCTIONS:
*       EXPORTS:
*               Fvd_BuildNewDb  - Build new database.
*       LOCALS:
*               ParseCDBRecord  - Parse cdb record in cdb file.
*               ParseXDBRecord  - Parse xdb record in xdb file.
*               ParseSDBRecord  - Parse sdb record in sdb file.
*               ParseWDBRecord  - Parse wdb record in wdb file.
*               Sort_CDBRec     - Callback function called by qsort function 
*                                   for sorting cdb & wdb record.
*               Sort_XDBRec     - Callback function called by qsort function
*                                   for sorting xdb record.
*               Sort_SDBRec     - Callback function called by qsort function
*                                   for sorting sdb record.
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>  /* for SEEK_SET */
#include <io.h>     /* for _taccess & _tsopen */
#include <fcntl.h>  /* for mode constant in _tsopen */
#include <share.h>  /* for share mode consant in _tsopen */
#include <time.h>   /* for _strdate */
#include <ctype.h>  /* for isxdigit */
#include <string.h>
#include "flatav-int.h"
#include "fvd.h"    /* for fvd structs */
#include "path.h"   /* for PathAddExt */
#include "crc32file.h"  /* for calculating checksum */
#include "crc32.h"  /* same as above */
#include "macro.h"  /* useful macro */
#include "debug.h"

#define WLOG    _T("C:\\flav_log.txt")

#define FLATAV_MARK "FlatAV"
#define DEFAULT_FVD_FILENAME    _T("main.fvd")

#define FVD_HEADER_SIZE         0x200
#define FVD_DIRECTORY_OFFSET    0x100
#define FVD_DATA_OFFSET         0x200

#define FVD_MAX_FILETYPE        10
#define FVD_TYPEGROUPDIR_SIZE   (FVD_MAX_FILETYPE * 8)

#define CDBREC_SIZE         40
#define WDBREC_SIZE         CDBREC_SIZE
#define XDBREC_SIZE         56  /* Without FileType */


#define CDB_TOKEN_COUNT     4
#define XDB_TOKEN_COUNT     6
#define SDB_TOKEN_COUNT     5
#define WDB_TOKEN_COUNT     CDB_TOKEN_COUNT
#define HDB_TOKEN_COUNT     4


typedef struct fvd_xdbrec_i_t
{
    char szMalwareName[31];
    unsigned char bMalwareType;
    char szFileOffset[16];
    unsigned int dwSignLength;
    unsigned int dwSignCRC32;
    unsigned int bFileType;  /* Not written to db file (just an information for creating type subdirectory) */
}
FVD_XDBREC_I, *LPFVD_XDBREC_I;

typedef struct fvd_sdbrec_i_t
{
    unsigned short wRecSize;    /* Real SDB rec size, FVD_MIN_SDBREC_SIZE + strlen(szSignHexStr) */
    char szMalwareName[31];
    unsigned char bMalwareType;
    char szFileOffset[16];
    unsigned char szSignHexStr[512+1];  /* Not all written to db file, only its length bytes written */
    unsigned int bFileType;   /* Not written to db file (just an information for creating type subdirectory) */
}
FVD_SDBREC_I, *LPFVD_SDBREC_I;


typedef struct fvd_hdbrec_t
{
    unsigned int dwDbMayorVersion;
    unsigned char bDbMinorVersion;
    unsigned short wMinAvMayorVersion;
    unsigned char bMinAvMinorVersion;
}
FVD_HDBREC, *LPFVD_HDBREC;

#ifdef _DEBUG   /* DEBUGGING STUFF */

static void PrintCDB (LPFVD_CDBREC pCDBRec, unsigned int count)
{
    unsigned int i;
    printf ("CDB Record\n\n");
    for (i = 0; i < count; ++i) {
        printf("#%d\n", i);
        printf("MW Name: %s\n", pCDBRec[i].szMalwareName);
        printf("MW Type: %d\n", pCDBRec[i].bMalwareType);
        printf("Size: %d\n", pCDBRec[i].dwFileSize);
        printf("CRC32: %x\n", pCDBRec[i].dwCRC32);
        printf("\n\n");
    }
}

static void PrintXDB (LPFVD_XDBREC_I pXDBRec, unsigned int count)
{
    unsigned int i;
    printf ("XDB Record\n\n");
    for (i = 0; i < count; ++i) {
        printf("#%d\n", i);
        printf("File Type: %d\n", pXDBRec[i].bFileType);
        printf("MW Name: %s\n", pXDBRec[i].szMalwareName);
        printf("MW Type: %d\n", pXDBRec[i].bMalwareType);
        printf("File Offset: %s\n", pXDBRec[i].szFileOffset);
        printf("Sign Length: %x\n", pXDBRec[i].dwSignLength);
        printf("Sign CRC32: %x\n", pXDBRec[i].dwSignCRC32);
        printf("\n\n");
    }
}

static void PrintSDB (LPFVD_SDBREC_I pSDBRec, unsigned int count)
{
    unsigned int i;
    printf ("SDB Record\n\n");
    for (i = 0; i < count; ++i) {
        /*unsigned int j, len;*/
        printf("#%d\n", i);
        printf("Written Record Size: %d\n", pSDBRec[i].wRecSize);
        printf("File Type: %d\n", pSDBRec[i].bFileType);
        printf("MW Name: %s\n", pSDBRec[i].szMalwareName);
        printf("MW Type: %d\n", pSDBRec[i].bMalwareType);
        printf("File Offset: %s\n", pSDBRec[i].szFileOffset);

        printf("Sign Hex String: %s\n", pSDBRec[i].szSignHexStr);
        /*len = pSDBRec[i].wRecSize - FVD_MIN_SDBREC_SIZE;
        for (j = 0; j < len; ++j)
            printf("%02x", pSDBRec[i].szSignHexStr[j]);
        printf("\n");*/

        printf("\n\n");
    }
}

#else

#define PrintCDB(p, c)  (0)
#define PrintXDB(p, c)  (0)
#define PrintSDB(p, c)  (0)

#endif  /* DEBUGGING STUFF */

static int Search_CDBRec (const void *key, const void *mid)
{
    return (int) (
        ((LPFVD_FIND_CWDBREC) key)->dwFileSize != ((LPFVD_CDBREC) mid)->dwFileSize ?
        ((LPFVD_FIND_CWDBREC) key)->dwFileSize - ((LPFVD_CDBREC) mid)->dwFileSize :
        ((LPFVD_FIND_CWDBREC) key)->dwCRC32 - ((LPFVD_CDBREC) mid)->dwCRC32 
    );
}

static int Sort_SDBRec (const void *a, const void *b)
{
    char* p;
    char* q;
    unsigned int x, y;

    /* XDB is sorted by bFileType and szFileOffset */
    if ( ((LPFVD_SDBREC_I) a)->bFileType < ((LPFVD_SDBREC_I) b)->bFileType )
        return -1;
    if ( ((LPFVD_SDBREC_I) a)->bFileType > ((LPFVD_SDBREC_I) b)->bFileType )
        return 1;
    /* if ( ((LPFVD_SDBREC_I) a)->bFileType == ((LPFVD_SDBREC_I) b)->bFileType ) */
    p = (((LPFVD_SDBREC_I) a)->szFileOffset);
    q = ((LPFVD_SDBREC_I) b)->szFileOffset;

    /* If both are hex number */
    if (isxdigit (p[0]) && isxdigit (q[0])) {
        x = strtoul (p, NULL, 16);
        y = strtoul (q, NULL, 16);

        if (x < y)
            return -1;
        if (x > y)
            return 1;
        return 0;
    }

    if (p[0] < q[0])
        return -1;
    if (p[0] > q[0])
        return 1;
    /* if (p[0] == q[0]) */
    /* If both are using section */
    if (p[0] == FVD_FO_SECTION && q[0] == FVD_FO_SECTION) {
        /* Compare section index, max index is 65535 but usually an application just has
           2 - 4 sections */
        if (p[1] < q[1])
            return -1;
        if (p[1] > q[1])
            return 1;
    }
    /* if (p[1] == q[1]) */
    /* Comparing szFileOffset that has '+' or '-' char */
    p = strchr (((LPFVD_SDBREC_I) a)->szFileOffset, '+');
    if (!p) {
        p = strchr (((LPFVD_SDBREC_I) a)->szFileOffset, '-');
        if (!p) {
            return _strcmpi (((LPFVD_SDBREC_I) a)->szFileOffset, ((LPFVD_SDBREC_I) b)->szFileOffset);
        }
    }

    q = strchr (((LPFVD_SDBREC_I) b)->szFileOffset, '+');
    if (!q) {
        q = strchr (((LPFVD_SDBREC_I) b)->szFileOffset, '-');
        if (!q) {
            return _strcmpi (((LPFVD_SDBREC_I) a)->szFileOffset, ((LPFVD_SDBREC_I) b)->szFileOffset);
        }
    }

    x = strtoul (p+1, NULL, 16);
    y = strtoul (q+1, NULL, 16);

    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

static int Sort_XDBRec (const void *a, const void *b)
{
    char* p;
    char* q;
    unsigned int x, y;

    /* XDB is sorted by bFileType and szFileOffset */
    if ( ((LPFVD_XDBREC_I) a)->bFileType < ((LPFVD_XDBREC_I) b)->bFileType )
        return -1;
    if ( ((LPFVD_XDBREC_I) a)->bFileType > ((LPFVD_XDBREC_I) b)->bFileType )
        return 1;
    /* if ( ((LPFVD_XDBREC_I) a)->bFileType == ((LPFVD_XDBREC_I) b)->bFileType ) */
    p = (((LPFVD_XDBREC_I) a)->szFileOffset);
    q = ((LPFVD_XDBREC_I) b)->szFileOffset;

    /* If both are hex number */
    if (isxdigit (p[0]) && isxdigit (q[0])) {
        x = strtoul (p, NULL, 16);
        y = strtoul (q, NULL, 16);

        if (x < y)
            return -1;
        if (x > y)
            return 1;
        return 0;
    }
    /* If both has a different first special char */
    if (p[0] < q[0])
        return -1;
    if (p[0] > q[0])
        return 1;
    /* if (p[0] == q[0]) */
    /* If both are using section */
    if (p[0] == FVD_FO_SECTION && q[0] == FVD_FO_SECTION) {
        /* Compare section index, max index is 65535 but usually an application just has
           2 - 4 sections */
        if (p[1] < q[1])
            return -1;
        if (p[1] > q[1])
            return 1;
    }
    /* if (p[1] == q[1]) */
    /* Comparing szFileOffset that has '+' or '-' char */
    p = strchr (((LPFVD_XDBREC_I) a)->szFileOffset, '+');
    if (!p) {
        p = strchr (((LPFVD_XDBREC_I) a)->szFileOffset, '-');
        if (!p) {
            return _strcmpi (((LPFVD_XDBREC_I) a)->szFileOffset, ((LPFVD_XDBREC_I) b)->szFileOffset);
        }
    }

    q = strchr (((LPFVD_XDBREC_I) b)->szFileOffset, '+');
    if (!q) {
        q = strchr (((LPFVD_XDBREC_I) b)->szFileOffset, '-');
        if (!q) {
            return _strcmpi (((LPFVD_XDBREC_I) a)->szFileOffset, ((LPFVD_XDBREC_I) b)->szFileOffset);
        }
    }

    x = strtoul (p+1, NULL, 16);
    y = strtoul (q+1, NULL, 16);

    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

static bool_t ParseSDBRecord (char* szLineString, LPFVD_SDBREC_I pSDBRec)
{
    int n = 0;
    char* p;

    /* Parse File Type */
    p = strtok (szLineString, ":");
    if (p) {
        pSDBRec->bFileType = atoi (p);
        if (pSDBRec->bFileType >= FVD_MAX_FILETYPE)
            return 0;
        ++n;
    }

    /* Parse Malware Name */
    p = strtok (NULL, ":");
    if (p) {
        strcpy (pSDBRec->szMalwareName, p);
        ++n;
    }

    /* Parse Malware Type */
    p = strtok (NULL, ":");
    if (p) {
        pSDBRec->bMalwareType = atoi (p);
        ++n;
    }

    /* Parse File Offset String */
    p = strtok (NULL, ":");
    if (p) {
        strcpy (pSDBRec->szFileOffset, strlwr (p));
        ++n;
    }

    /* Parse Sign Hex String */
    p = strtok (NULL, "\n");
    if (p) {
        size_t len = strlen (p);
        /*unsigned int i, j;
        for (i = 0, j = 0; i < len; i+=2, ++j) {
            char szByte[3];
            strncpy (szByte, &p[i], 2);
            szByte[2] = '\0';
            pSDBRec->szSignHexStr[j] = (unsigned char) strtoul (szByte, NULL, 16);
        }
        pSDBRec->szSignHexStr[j] = '\0';*/

        strncpy (pSDBRec->szSignHexStr, p, sizeof(pSDBRec->szSignHexStr));
        pSDBRec->szSignHexStr[512] = '\0';

        pSDBRec->wRecSize = FVD_MIN_SDBREC_SIZE + (int) len;
        ++n;
    }

    if (n == SDB_TOKEN_COUNT)
        return 1;
    else
        return 0;
}

static bool_t ParseXDBRecord (char* szLineString, LPFVD_XDBREC_I pXDBRec)
{
    int n = 0;
    char* p;

    /* Parse File Type */
    p = strtok (szLineString, ":");
    if (p) {
        pXDBRec->bFileType = atoi (p);
        if (pXDBRec->bFileType >= FVD_MAX_FILETYPE)
            return 0;
        ++n;
    }

    /* Parse Malware Name */
    p = strtok (NULL, ":");
    if (p) {
        strcpy (pXDBRec->szMalwareName, p);
        ++n;
    }

    /* Parse Malware Type */
    p = strtok (NULL, ":");
    if (p) {
        pXDBRec->bMalwareType = atoi (p);
        ++n;
    }

    /* Parse File Offset string */
    p = strtok (NULL, ":");
    if (p) {
        strcpy (pXDBRec->szFileOffset, strlwr (p));
        ++n;
    }

    /* Parse Sign Length */
    p = strtok (NULL, ":");
    if (p) {
        pXDBRec->dwSignLength = strtoul (p, NULL, 0);
        ++n;
    }

    /* Parse Sign CRC32 */
    p = strtok (NULL, "\n");
    if (p) {
        pXDBRec->dwSignCRC32 = strtoul (p, NULL, 16);
        ++n;
    }

    if (n == XDB_TOKEN_COUNT)
        return 1;
    else
        return 0;
}


static int Sort_CDBRec (const void *a, const void *b)
{
    /* Round filesize into kb size plus one */
    unsigned dwFileSizeA, dwFileSizeB;
    dwFileSizeA = (((LPFVD_CDBREC) a)->dwFileSize / 1000) + 1;
    dwFileSizeB = (((LPFVD_CDBREC) b)->dwFileSize / 1000) + 1;

    /* CDB is sorted by dwFileSize and dwCrc32 */
    if (dwFileSizeA < dwFileSizeB)
        return -1;
    if (dwFileSizeA > dwFileSizeB)
        return 1;
    /* if (dwFileSizeA == dwFileSizeB) */
    if ( ((LPFVD_CDBREC) a)->dwCRC32 < ((LPFVD_CDBREC) b)->dwCRC32 )
        return -1;
    if ( ((LPFVD_CDBREC) a)->dwCRC32 > ((LPFVD_CDBREC) b)->dwCRC32 )
        return 1;
    return 0;
}

static bool_t ParseCDBRecord (char* szLineString, LPFVD_CDBREC pCDBRec)
{
    int n = 0;
    char* p;

    /* Parse Malware Name */
    p = strtok (szLineString, ":");
    if (p) {
        strcpy (pCDBRec->szMalwareName, p);
        ++n;
    }

    /* Parse Malware Type */
    p = strtok (NULL, ":");
    if (p) {
        pCDBRec->bMalwareType = atoi (p);
        ++n;
    }

    /* Parse File Size */
    p = strtok (NULL, ":");
    if (p) {
        pCDBRec->dwFileSize = strtoul (p, NULL, 0);
        ++n;
    }

    /* Parse CRC32 */
    p = strtok (NULL, "\n");
    if (p) {
        pCDBRec->dwCRC32 = strtoul (p, NULL, 16);
        ++n;
    }

    if (n == CDB_TOKEN_COUNT)
        return 1;
    else
        return 0;
}

static bool_t ParseHDBRecord (char* szLineString, LPFVD_HDBREC pHDBRec)
{
    int n = 0;
    char* p;

    /* Parse DbMayorVersion */
    p = strtok (szLineString, ":");
    if (p) {
        pHDBRec->dwDbMayorVersion = atoi (p);
        ++n;
    }

    /* Parse DbMinorVersion */
    p = strtok (NULL, ":");
    if (p) {
        pHDBRec->bDbMinorVersion = atoi (p);
        ++n;
    }

    /* Parse wMinAvMayorVersion */
    p = strtok (NULL, ":");
    if (p) {
        pHDBRec->wMinAvMayorVersion = atoi (p);
        ++n;
    }

    /* Parse bMinAvMinorVersion */
    p = strtok (NULL, "\n");
    if (p) {
        pHDBRec->bMinAvMinorVersion = atoi (p);
        ++n;
    }

    if (n == HDB_TOKEN_COUNT)
        return 1;
    else
        return 0;
}

/* EXPORTED FUNCTIONS */

/****************************************************************************
*
*   FUNCTION:   Fvd_BuildNewDb
*
*   PURPOSE:    Build new database.
*
*   PARAMS:
*               szCDBFilePath       - cdb file path.
*               szXDBFilePath       - xdb file path.
*               szSDBFilePath       - sdb file path.
*               szWDBFilePath       - wdb file path.
*               dwDbMayorVersion    - mayor version of database.
*               bDbMinorVersion     - minor version of database.
*               wMinAvMayorVersion  - minimum mayor version of AV for using
*                                       current database.
*               bMinAvMinorVersion  - minimum minor version of AV for using
*                                       current database.
*               szFVDFilePath       - output database file path.
*
*   RETURNS:    int - error code (0 for no error).
*
\****************************************************************************/
int Fvd_BuildNewDb (const _TCHAR* szHDBFilePath, 
                    const _TCHAR* szCDBFilePath, const _TCHAR* szXDBFilePath, 
                    const _TCHAR* szSDBFilePath, const _TCHAR* szWDBFilePath,
                    _TCHAR* szFVDFilePath)
{
    int nRet = 0;
    FILE* fWriteDb;
    FILE* fReadDb;
    char szBuffer[1024];
    _TCHAR szTempFileName[13];
    FVD_HDBREC hdbRec;
    unsigned dwSignCount = 0;   /* For saving total of signature */
    unsigned long dwCrc32Checksum;
    unsigned int n = 0;
    unsigned dwCDBTblSize = 0, dwXDBTblSize = 0, dwSDBTblSize = 0, dwWDBTblSize = 0;
    unsigned int uType, uCount, uOffset;    /* For creating file type group directory */

    /* Create temp db file name */
    _ttmpnam (szTempFileName);
    /*do {
        _TCHAR _szIncNum[3];
        _tcscpy (szTempFileName, _T("_00"));
        _itot (++n, _szIncNum, 10);
        _tcscat (szTempFileName, _szIncNum);
        _tcscat (szTempFileName, _T(".fv~"));
    } while (_taccess (szTempFileName, 0) == 0);*/

    
    /* Create temp db file */
    fWriteDb = _tfopen (szTempFileName, _T("w+b"));
    if (fWriteDb == NULL)
        return FL_ETEMPF;

    if (szHDBFilePath != NULL) {
        if (_taccess (szHDBFilePath, 4) == -1) {
            nRet = FVD_EACCESS_HDB;
LRetError:
            fclose (fWriteDb);
            _tremove (szTempFileName);
            return nRet; 
        }
        // Open hdb file
        fReadDb = _tfopen (szHDBFilePath, _T("r"));
        if (fReadDb == NULL) {
            nRet = FVD_EOPEN_HDB;
            goto LRetError;
        }

        if (feof (fReadDb) || 
            !fgets (szBuffer, sizeof(szBuffer), fReadDb) ||
            strlen (szBuffer) <= HDB_TOKEN_COUNT) {
            fclose (fReadDb);
            nRet = FVD_EOPEN_HDB;
            goto LRetError;
        }
        else {
            if (!ParseHDBRecord (szBuffer, &hdbRec)) {
                fclose (fReadDb);
                nRet = FVD_EINV_HDB;
                goto LRetError;
            }
        }

        fclose (fReadDb);
    }
    else {
        nRet = FL_ENULLARG;
        goto LRetError;
    }

    /* Write Mark "FlatAV" (6 bytes), exclude null */
    fwrite (FLATAV_MARK, 1, sizeof(FLATAV_MARK)-1, fWriteDb);

    /* Write release date (8 bytes), exclude null */
    _strdate (szBuffer);
    fwrite (szBuffer, 1, 8, fWriteDb);
    
    /* Write MayorDbVersion (4 bytes) and MinorDbVersion (1 byte) */
    fwrite (&hdbRec.dwDbMayorVersion, 1, sizeof(hdbRec.dwDbMayorVersion), fWriteDb);
    fwrite (&hdbRec.bDbMinorVersion, 1, sizeof(hdbRec.bDbMinorVersion), fWriteDb);

    /* Write MinimalMayorAvVersion (2 bytes) and MinimalMinorAvVersion (1 byte) */
    fwrite (&hdbRec.wMinAvMayorVersion, 1, sizeof(hdbRec.wMinAvMayorVersion), fWriteDb);
    fwrite (&hdbRec.bMinAvMinorVersion, 1, sizeof(hdbRec.bMinAvMinorVersion), fWriteDb);

    /* Goto data directory area (0x100) */
    fseek (fWriteDb, FVD_DIRECTORY_OFFSET+8*0, SEEK_SET);

    /*
    //
    // Parse file cdb (if exists)
    //
    */

    if (szCDBFilePath != NULL) {
        unsigned dwCDBRecCount;
        LPFVD_CDBREC aCDBRec;

        if (_taccess(szCDBFilePath, 4) == -1) {
            nRet = FVD_EACCESS_CDB;
            goto LRetError;
        }

        // Open cdb file
        fReadDb = _tfopen (szCDBFilePath, _T("r"));
        if (fReadDb == NULL) {
            nRet = FVD_EOPEN_CDB;
            goto LRetError;
        }

        // Find how many of valid record inside the cdb file
        dwCDBRecCount = 0;
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > CDB_TOKEN_COUNT)
                ++dwCDBRecCount;
        }

        // If no valid record found, exit with error
        if (dwCDBRecCount == 0) {
LInvalidCDBFormat:
            fclose (fReadDb);
            nRet = FVD_EINV_CDB;
            goto LRetError;
        }

        // Write CDBTblOffset
        n = FVD_DATA_OFFSET;
        fwrite (&n, 1, 4, fWriteDb);

        // Write CDBTblRecCount
        dwCDBTblSize = dwCDBRecCount * CDBREC_SIZE;
        fwrite (&dwCDBRecCount, 1, 4, fWriteDb); /* dwCDBTblSize -> dwCDBRecCount */

        dwSignCount = dwCDBRecCount;	// Update signcount

        // After we found out the record count, allocate storage for cdb record array
        aCDBRec = (LPFVD_CDBREC) malloc (dwCDBRecCount * sizeof(FVD_CDBREC));

        // Parse cdb record from the file into aCDBRec
        n = 0;
        fseek (fReadDb, 0, SEEK_SET);
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > CDB_TOKEN_COUNT) {
                if (!ParseCDBRecord (szBuffer, &aCDBRec[n]))
                    goto LInvalidCDBFormat;
                ++n;
            }
        }

        qsort (aCDBRec, dwCDBRecCount, sizeof(aCDBRec[0]), Sort_CDBRec);	// Sort it

        PrintCDB (aCDBRec, dwCDBRecCount);

        // Write all cdb record
        fseek (fWriteDb, FVD_DATA_OFFSET, SEEK_SET);
        fwrite (aCDBRec, 1, dwCDBRecCount * sizeof(FVD_CDBREC), fWriteDb);

        // Free resources
        free (aCDBRec);
        fclose (fReadDb);
    }
    else {
        // If cdb file doesn't exist, set CDBTblOffset and CDBTblSize to 0.
        n = 0;
        fwrite (&n, 1, 4, fWriteDb);
        fwrite (&n, 1, 4, fWriteDb);
    }

    // Goto XDBTblOffset
    fseek (fWriteDb, FVD_DIRECTORY_OFFSET+8*1, SEEK_SET);


    /*
    //
    // Parse file xdb (if exists)
    //
    */

    if (szXDBFilePath != NULL) {
        unsigned int dwXDBRecCount;
        LPFVD_XDBREC_I aXDBRec;

        if (_taccess(szXDBFilePath, 4) == -1) {
            nRet = FVD_EACCESS_XDB;
            goto LRetError;
        }

        // Open xdb file
        fReadDb = _tfopen (szXDBFilePath, _T("r"));
        if (fReadDb == NULL) {
            nRet = FVD_EOPEN_XDB;
            goto LRetError;
        }

        // Find how many of valid record inside the xdb file
        dwXDBRecCount = 0;
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > XDB_TOKEN_COUNT)
                ++dwXDBRecCount;
        }

        // If no valid record found, exit with error
        if (dwXDBRecCount == 0) {
LInvalidXDBFormat:
            fclose (fReadDb);
            nRet = FVD_EINV_XDB;
            goto LRetError;
        }

        // Write XDBTblOffset
        n = FVD_DATA_OFFSET + dwCDBTblSize;
        fwrite (&n, 1, 4, fWriteDb);

        // Write XDBTblRecCount
        dwXDBTblSize = dwXDBRecCount * XDBREC_SIZE + FVD_TYPEGROUPDIR_SIZE;
        fwrite (&dwXDBRecCount, 1, 4, fWriteDb);

        dwSignCount += dwXDBRecCount;	// Update signcount

        // After we found out the record count, allocate storage for xdb record array
        aXDBRec = (LPFVD_XDBREC_I) malloc (dwXDBRecCount * sizeof(FVD_XDBREC_I));

        // Parse xdb record from the file into aXDBRec
        n = 0;
        fseek (fReadDb, 0, SEEK_SET);
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > XDB_TOKEN_COUNT) {
                if (!ParseXDBRecord (szBuffer, &aXDBRec[n]))
                    goto LInvalidXDBFormat;
                ++n;
            }
        }

        qsort (aXDBRec, dwXDBRecCount, sizeof(aXDBRec[0]), Sort_XDBRec);	// Sort it

        PrintXDB (aXDBRec, dwXDBRecCount);

        /* Create file type group directory for XDB table */
        fseek (fWriteDb, FVD_DATA_OFFSET + dwCDBTblSize, SEEK_SET);

        n = 0;
        uType = uCount = 0;
        while (1) {
            /* If the current type not exists in active XDB record, then fill groupdir with 0 */
            while ((n >= dwXDBRecCount || aXDBRec[n].bFileType) != uType && uType < FVD_MAX_FILETYPE) {
                uCount = 0;
                fwrite (&uCount, 1, 4, fWriteDb);
                fwrite (&uCount, 1, 4, fWriteDb);
                ++uType;
            }
            if (uType == FVD_MAX_FILETYPE)
                break;

            uOffset = FVD_DATA_OFFSET + dwCDBTblSize + FVD_TYPEGROUPDIR_SIZE + n * XDBREC_SIZE;
            fwrite (&uOffset, 1, 4, fWriteDb);   /* Write offset to table */
            uCount = n;

            while (n < dwXDBRecCount && aXDBRec[n].bFileType == uType) ++n;    /* Skip same type found */
            uCount = n - uCount;

            fwrite (&uCount, 1, 4, fWriteDb);    /* Write count of record according to type */
            ++uType;    /* Forward to next type */
        }

        // Write all xdb record
        fseek (fWriteDb, FVD_DATA_OFFSET + dwCDBTblSize + FVD_TYPEGROUPDIR_SIZE, SEEK_SET);
        //fwrite (aXDBRec, 1, dwXDBRecCount * sizeof(FVD_XDBREC_I), fWriteDb);
        for (n = 0; n < dwXDBRecCount; ++n)
            fwrite (aXDBRec[n].szMalwareName, 1, XDBREC_SIZE, fWriteDb);

        // Free resources
        free (aXDBRec);
        fclose (fReadDb);
    }
    else {
        // If xdb file doesn't exist, set XDBTblOffset and XDBTblSize to 0.
        n = 0;
        fwrite (&n, 1, 4, fWriteDb);
        fwrite (&n, 1, 4, fWriteDb);
    }

    // Goto SDBTblOffset
    fseek (fWriteDb, FVD_DIRECTORY_OFFSET+8*2, SEEK_SET);

    /*
    //
    // Parse file sdb (if exists)
    //
    */

    if (szSDBFilePath != NULL) {
        unsigned int dwSDBRecCount;
        LPFVD_SDBREC_I aSDBRec;

        if (_taccess(szSDBFilePath, 4) == -1) {
            nRet = FVD_EACCESS_SDB;
            goto LRetError;
        }

        // Open sdb file
        fReadDb = _tfopen (szSDBFilePath, _T("r"));
        if (fReadDb == NULL) {
            nRet = FVD_EOPEN_SDB;
            goto LRetError;
        }

        // Find how many of valid record inside the sdb file
        dwSDBRecCount = 0;
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > SDB_TOKEN_COUNT)
                ++dwSDBRecCount;
        }

        // If no valid record found, exit with error
        if (dwSDBRecCount == 0) {
LInvalidSDBFormat:
            fclose (fReadDb);
            nRet = FVD_EINV_SDB;
            goto LRetError;
        }

        // Write SDBTblOffset
        n = FVD_DATA_OFFSET + dwCDBTblSize + dwXDBTblSize;
        fwrite (&n, 1, 4, fWriteDb);

        //dwSDBTblSize = dwSDBRecCount * sizeof(FVD_SDBREC_I);
        //fwrite (&dwSDBTblSize, 1, 4, fWriteDb);

        dwSignCount += dwSDBRecCount;   // Update signcount

        // After we found out the record count, allocate storage for sdb record array
        aSDBRec = (LPFVD_SDBREC_I) malloc (dwSDBRecCount * sizeof(FVD_SDBREC_I));

        // Parse sdb record from the file into aSDBRec
        n = 0;
        fseek (fReadDb, 0, SEEK_SET);
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > SDB_TOKEN_COUNT) {
                if (!ParseSDBRecord (szBuffer, &aSDBRec[n]))
                    goto LInvalidSDBFormat;
                ++n;
            }
        }

        // Calculate SDBTblSize
        dwSDBTblSize = 0;
        for (n = 0; n < dwSDBRecCount; ++n)
            dwSDBTblSize += aSDBRec[n].wRecSize;
        dwSDBTblSize += FVD_TYPEGROUPDIR_SIZE;

        // Write SDBTblRecCount
        fwrite (&dwSDBRecCount, 1, 4, fWriteDb);

        qsort (aSDBRec, dwSDBRecCount, sizeof(aSDBRec[0]), Sort_SDBRec);    // Sort it

        PrintSDB (aSDBRec, dwSDBRecCount);

        /* Create file type group directory for SDB table */
        /* Group directory consist of 2 fields: Offset (4 bytes) and Record Count (4 bytes) */
        fseek (fWriteDb, FVD_DATA_OFFSET + dwCDBTblSize + dwXDBTblSize, SEEK_SET);

        n = 0;  /* current record index */
        uType = uCount = 0;
        uOffset = FVD_DATA_OFFSET + dwCDBTblSize + dwXDBTblSize + FVD_TYPEGROUPDIR_SIZE;
        /* Iterate each type until FVD_MAX_FILETYPE */
        while (1) {
            /* If the current type not exists in active SDB record, then fill groupdir with 0 */
            while ((n >= dwSDBRecCount || aSDBRec[n].bFileType != uType) && uType < FVD_MAX_FILETYPE) {
                uCount = 0;
                fwrite (&uCount, 1, 4, fWriteDb);
                fwrite (&uCount, 1, 4, fWriteDb);
                ++uType;
            }
            if (uType == FVD_MAX_FILETYPE)
                break;

            fwrite (&uOffset, 1, 4, fWriteDb);  /* Write offset of current type */
            uCount = n; /* Save current record index */

            /* Calculate the record count of current type */
            while (n < dwSDBRecCount && aSDBRec[n].bFileType == uType) {
                uOffset += aSDBRec[n].wRecSize;
                ++n;
            }
            uCount = n - uCount;    /* The differences is the record count */

            fwrite (&uCount, 1, 4, fWriteDb);   /* Write record count */
            ++uType;    /* Go to next type */
        }

        // Write all sdb record
        fseek (fWriteDb, FVD_DATA_OFFSET + dwCDBTblSize + dwXDBTblSize + FVD_TYPEGROUPDIR_SIZE, SEEK_SET);
        for (n = 0; n < dwSDBRecCount; ++n)
            fwrite (&aSDBRec[n].wRecSize, 1, aSDBRec[n].wRecSize, fWriteDb);

        // Free resources
        free (aSDBRec);
        fclose (fReadDb);
    }
    else {
        // If sdb file doesn't exist, set SDBTblOffset and SDBTblSize to 0.
        n = 0;
        fwrite (&n, 1, 4, fWriteDb);
        fwrite (&n, 1, 4, fWriteDb);
    }

    // Goto WDBTblOffset
    fseek (fWriteDb, FVD_DIRECTORY_OFFSET+8*3, SEEK_SET);

    /*
    //
    // Parse file wdb (if exists)
    //
    */

    if (szWDBFilePath != NULL) {
        unsigned dwWDBRecCount;
        LPFVD_WDBREC aWDBRec;

        if (_taccess(szWDBFilePath, 4) == -1) {
            nRet = FVD_EACCESS_WDB;
            goto LRetError;
        }

        // Open wdb file
        fReadDb = _tfopen (szWDBFilePath, _T("r"));
        if (fReadDb == NULL) {
            nRet = FVD_EOPEN_WDB;
            goto LRetError;
        }

        // Find how many of valid record inside the wdb file
        dwWDBRecCount = 0;
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > WDB_TOKEN_COUNT)
                ++dwWDBRecCount;
        }

        // If no valid record found, exit with error
        if (dwWDBRecCount == 0) {
LInvalidWDBFormat:
            fclose (fReadDb);
            nRet = FVD_EINV_WDB;
            goto LRetError;
        }

        // Write WDBTblOffset
        n = FVD_DATA_OFFSET + dwCDBTblSize + dwXDBTblSize + dwSDBTblSize;
        fwrite (&n, 1, 4, fWriteDb);

        // Write WDBTblRecCount
        dwWDBTblSize = dwWDBRecCount * WDBREC_SIZE;
        fwrite (&dwWDBRecCount, 1, 4, fWriteDb);

        dwSignCount += dwWDBRecCount;   // Update signcount

        // After we found out the record count, allocate storage for wdb record array
        aWDBRec = (LPFVD_WDBREC) malloc (dwWDBRecCount * sizeof(FVD_WDBREC));

        // Parse cdb record from the file into aWDBRec
        n = 0;
        fseek (fReadDb, 0, SEEK_SET);
        while (!feof (fReadDb) && fgets (szBuffer, sizeof(szBuffer), fReadDb)) {
            if (strlen (szBuffer) > WDB_TOKEN_COUNT) {
                if (!ParseCDBRecord (szBuffer, &aWDBRec[n]))
                    goto LInvalidWDBFormat;
                ++n;
            }
        }

        qsort (aWDBRec, dwWDBRecCount, sizeof(aWDBRec[0]), Sort_CDBRec);    // Sort it

        //PrintWDB (aWDBRec, dwWDBRecCount);

        // Write all wdb record
        fseek (fWriteDb, FVD_DATA_OFFSET+dwCDBTblSize+dwXDBTblSize+dwSDBTblSize, SEEK_SET);
        fwrite (aWDBRec, 1, dwWDBRecCount * sizeof(FVD_WDBREC), fWriteDb);

        // Free resources
        free (aWDBRec);
        fclose (fReadDb);
    }
    else {
        // If wdb file doesn't exist, set WDBTblOffset and WDBTblSize to 0.
        n = 0;
        fwrite (&n, 1, 4, fWriteDb);
        fwrite (&n, 1, 4, fWriteDb);
    }

    // Calculate checksum
    fseek (fWriteDb, FVD_DATA_OFFSET, SEEK_SET);
    if (Crc32File_Handle (_fileno (fWriteDb), 
        dwCDBTblSize + dwXDBTblSize + dwSDBTblSize + dwWDBTblSize, &dwCrc32Checksum) != 0) {
        nRet =  FVD_EWRITE_CHECKSUM;
        goto LRetError;
    }

    // Write signature count
    fseek (fWriteDb, 22, SEEK_SET);
    fwrite (&dwSignCount, 1, 4, fWriteDb);

    // Write checksum
    fwrite (&dwCrc32Checksum, 1, 4, fWriteDb);

    fclose (fWriteDb);   // Close file to write, that means writing is done

    /* Rename temp db filename to new db filename if user specified the name
       else uses default name ("main.fvd") */
    if (szFVDFilePath != NULL) {
        PathAddExt (szFVDFilePath, _T(".fvd"));

        // If filename is already exists, delete it
        if (_taccess (szFVDFilePath, 0) == 0)
             _tremove (szFVDFilePath);

        if (_trename (szTempFileName, szFVDFilePath) == -1) {
            _tremove (szTempFileName);
            return FVD_EWRITE_FVD;
        }
    }
    else {
        // If default filename is already exists, delete it
        if (_taccess (DEFAULT_FVD_FILENAME, 2) == 0)
            _tremove (DEFAULT_FVD_FILENAME);

        if (_trename (szTempFileName, DEFAULT_FVD_FILENAME) == -1) {
            _tremove (szTempFileName);
            return FVD_EWRITE_FVD;
        }
    }

    return 0;
}

int Fvd_LoadDb (_TCHAR* szDbFilePath, LPFVD_BLOCK lpFvdBlock)
{
    int handle;

    lpFvdBlock->data = NULL;    // prevent exception when Fvd_LoadDb failed and user call Fvd_UnloadDb
    //if (_taccess (szDbFilePath, 0) != 0)
    //    return E_CANT_FOUND_FVD_FILE;

    handle = _tsopen (szDbFilePath, _O_RDONLY | _O_BINARY, _SH_DENYWR);
    if (handle == -1)
        return FL_EOPEN;
    
    lpFvdBlock->size = _filelength (handle);
    lpFvdBlock->data = (char*) malloc (lpFvdBlock->size);
    if (lpFvdBlock->data == NULL)
        return FL_ENOMEM;

    if (_read (handle, lpFvdBlock->data, lpFvdBlock->size) == -1)
        return FL_EBADF;

    _close (handle);
    return 0;
}

void Fvd_ParseHeader (LPFVD_HEADER lpFvdHeader, const LPFVD_BLOCK lpFvdBlock)
{
    // Parse mark (6 bytes)
    memcpy (lpFvdHeader->szMark, lpFvdBlock->data, 6);
    lpFvdHeader->szMark[6] = '\0';

    // Parse release date (8 bytes)
    memcpy (lpFvdHeader->szReleaseDate, &lpFvdBlock->data[6], 8);
    lpFvdHeader->szReleaseDate[8] = '\0';

    // Parse DbMayorVersion (4 bytes) & DbMinorVersion (1 byte)
    lpFvdHeader->dwDbMayorVersion = PTR(unsigned int, lpFvdBlock->data[14]);
    lpFvdHeader->bDbMinorVersion = PTR(unsigned char, lpFvdBlock->data[18]);

    // Parse MinimalAvMayorVersion (2 bytes) & MinimalAvMinorVersion (1 byte)
    lpFvdHeader->wMinAvMayorVersion = PTR(unsigned short, lpFvdBlock->data[19]);
    lpFvdHeader->bMinAvMinorVersion = PTR(unsigned char, lpFvdBlock->data[21]);

    // Parse Sign count (4 bytes)
    lpFvdHeader->dwSignCount = PTR(unsigned int, lpFvdBlock->data[22]);

    // Parse checksum (4 bytes)
    lpFvdHeader->dwCrc32Checksum = PTR(unsigned int, lpFvdBlock->data[26]);
}

int Fvd_ValidateDb (const LPFVD_BLOCK lpFvdBlock, unsigned short wCurAvMayorVersion, 
                     unsigned char bCurAvMinorVersion)
{
    char szMark[6+1];
    unsigned int dwChecksum1, dwChecksum2;
    unsigned short wDbMinAvMayorVer;
    unsigned char bDbMinAvMinorVer;

    memcpy (szMark, lpFvdBlock->data, 6);
    szMark[6] = '\0';
    if (strcmp (szMark, FLATAV_MARK) != 0)
        return FVD_EINV_MARK;

    if (wCurAvMayorVersion != 0) {
        wDbMinAvMayorVer = PTR(unsigned short, lpFvdBlock->data[19]);
        if (wDbMinAvMayorVer > wCurAvMayorVersion)
            return FVD_EMIN_AV_VER;
        if (wDbMinAvMayorVer == wCurAvMayorVersion && bCurAvMinorVersion != 0) {
            bDbMinAvMinorVer = PTR(unsigned char, lpFvdBlock->data[21]);
            if (bDbMinAvMinorVer > bCurAvMinorVersion)
                return FVD_EMIN_AV_VER;
        }
    }

    dwChecksum1 = PTR(unsigned int, lpFvdBlock->data[26]);
    dwChecksum2 = Crc32_Asm (&lpFvdBlock->data[0x200], lpFvdBlock->size - 0x200, 0);
    if (dwChecksum1 != dwChecksum2)
        return FVD_EINV_CHECKSUM;
    return 0;
}

void Fvd_UnloadDb (LPFVD_BLOCK lpFvdBlock) 
{
    if (lpFvdBlock->data != NULL) 
        free (lpFvdBlock->data);
}

void* Fvd_GetTblPtr (unsigned int dwTbl, int iFileType, LPFVD_BLOCK lpFvdBlock)
{
    unsigned int dwTblOffset;
    if (dwTbl > 3)
        return NULL;

    /* Check if Table exists */
    dwTblOffset = PTR(unsigned int, lpFvdBlock->data[FVD_DIRECTORY_OFFSET+8*dwTbl]);
    if (dwTblOffset == 0)
        return NULL;

    if (dwTbl == FVD_CDBTBL || dwTbl == FVD_WDBTBL)
        return &lpFvdBlock->data[dwTblOffset];

    if (iFileType >= FVD_MAX_FILETYPE)
        return NULL;

    return &lpFvdBlock->data[PTR(unsigned int, lpFvdBlock->data[dwTblOffset+8*iFileType])];
}

unsigned int Fvd_GetTblRecCount (unsigned int dwTbl, int iFileType, LPFVD_BLOCK lpFvdBlock)
{
    unsigned int dwTblOffset;
    //return dwTbl <= 3 ? PTR(unsigned int, lpFvdBlock->data[FVD_DIRECTORY_OFFSET+8*dwTbl+4]) : 0;
    if (dwTbl > 3)
        return 0;

    if (dwTbl == FVD_CDBTBL || dwTbl == FVD_WDBTBL || iFileType == -1)
        return PTR(unsigned int, lpFvdBlock->data[FVD_DIRECTORY_OFFSET+8*dwTbl+4]);

    if (iFileType >= FVD_MAX_FILETYPE)
        return 0;

    dwTblOffset = PTR(unsigned int, lpFvdBlock->data[FVD_DIRECTORY_OFFSET+8*dwTbl]);
    return PTR(unsigned int, lpFvdBlock->data[dwTblOffset+8*iFileType+4]);
}

bool_t Fvd_LookupCWDB (LPFVD_FIND_CWDBREC lpFvdFindCwdbRec, LPFVD_BLOCK lpFvdBlock)
{
    LPFVD_CDBREC aDbRec;
    unsigned int dwDBTblOffset, dwDBTblCount;
    int n;

    /* Determine table type */
    if (lpFvdFindCwdbRec->bTblType == 0)
        n = 0;
    else
        n = 3;

    /* Check if CDB/WDB Table exists */
    dwDBTblOffset = PTR(unsigned int, lpFvdBlock->data[FVD_DIRECTORY_OFFSET + 8 * n]);
    if (dwDBTblOffset == 0)
        return 0;

    dwDBTblCount = PTR(unsigned int, lpFvdBlock->data[FVD_DIRECTORY_OFFSET + 8 * n + 4]);
    if (dwDBTblCount == 0)
        return 0;

    /* Goto CDB/WDB Table in lpFvdBlock */
    aDbRec = (LPFVD_CDBREC) &lpFvdBlock->data[dwDBTblOffset];
    
    /* Search CDB/WDB record */
    aDbRec = (LPFVD_CDBREC) bsearch (lpFvdFindCwdbRec, aDbRec, dwDBTblCount, 
        sizeof(FVD_CDBREC), Search_CDBRec);
    if (aDbRec) {
        /* If found, get malware name and malware type into lpFvdBlock */
        strcpy (lpFvdBlock->szMalwareName, aDbRec->szMalwareName);
        lpFvdBlock->bMalwareType = aDbRec->bMalwareType;
        return 1;
    }

    return 0;
}