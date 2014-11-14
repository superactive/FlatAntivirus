/****************************************************************************\
*
*	For processing archive files
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include "stdafx.h"
#include "FlatAVGUI.h"

#include "ArchiveOp.h"
#include "FileOp.h"

#include "zip.h"
#include "unzip.h"

BOOL CArchiveOp::IsArchive (LPCTSTR szFilePathName)
{
    int handle, nRet = ARC_NONE;
    char buffer[8];
    handle = _topen (szFilePathName, _O_RDONLY | _O_BINARY);
	if (handle == -1) {
        return -1;
	}

    if (_read (handle, buffer, sizeof(buffer)) > 0) {
        if (PTR(unsigned int, buffer[0]) == ZIP_SIGNATURE)
            nRet = ARC_ZIP;
        else if (PTR(unsigned short, buffer[0]) == GZIP_SIGNATURE)
            nRet = ARC_GZIP;
        else if (memcmp (buffer, SEVENZIP_SIGNATURE, sizeof(SEVENZIP_SIGNATURE)) == 0)
            nRet = ARC_7Z;
    }

    _close (handle);
    return (nRet != ARC_NONE);
}

int CArchiveOp::GetArchiveType (LPCTSTR szFilePathName)
{
    int handle, ret = ARC_NONE;
    char buffer[8];
    handle = _topen (szFilePathName, _O_RDONLY | _O_BINARY);
    if (handle == -1)
        return -1;

    if (_read (handle, buffer, sizeof(buffer)) > 0) {
        if (PTR(unsigned int, buffer[0]) == ZIP_SIGNATURE)
            ret = ARC_ZIP;
        else if (PTR(unsigned short, buffer[0]) == GZIP_SIGNATURE)
            ret = ARC_GZIP;
        else if (memcmp (buffer, SEVENZIP_SIGNATURE, sizeof(SEVENZIP_SIGNATURE)) == 0)
            ret = ARC_7Z;
    }

    _close (handle);
    return ret;
}

int CArchiveOp::ScanArchiveFile (LPCTSTR szFilePathName)
{
    int ret = FL_CLEAN;
    m_uScanCount = 0;
    switch (GetArchiveType(szFilePathName))
    {
    case ARC_ZIP:
        ret = ScanZipFile (szFilePathName);
        break;
    default:
        ret = FL_EARC;
    }
    return ret;
}

int CArchiveOp::DeleteFileInArchive (LPCTSTR szArchiveFPN, BOOL bDelPermanent, 
                                     BOOL bDelLockedFiles, LPCTSTR szFileNameInArchive)
{
    int ret = FL_SUCCESS;

    if (_taccess (szArchiveFPN, 0) == -1)
        return ret;

    switch (GetArchiveType(szArchiveFPN))
    {
    case ARC_ZIP:
        {
#ifdef _UNICODE
            char szFileNameInArchiveA[MAX_PATH];
            ::WideCharToMultiByte (CP_ACP, 0, szFileNameInArchive, -1, szFileNameInArchiveA,
                MAX_PATH, NULL, NULL);
            ret = DeleteFileInZip (szArchiveFPN, szFileNameInArchiveA, bDelPermanent);
#else
            ret = DeleteFileInZip (szArchiveFPN, szFileNameInArchive, bDelPermanent);
#endif
        }
        break;

    case ARC_RAR:
    default:
        ret = FL_ENOTSUPT;
    }

    if (ret == FL_EWRITE && bDelLockedFiles) {
        // TODO: The file might be locked, try delete it at next system restart.
    }
    return ret;
}

int CArchiveOp::MoveRenameFileInArchive (LPCTSTR szArchiveFPN, LPCTSTR szDestFolderPath, 
                                         BOOL bRenFile, BOOL bMoveRenLockedFiles,
                                         LPCTSTR szFileNameInArchive)
{
    int ret = FL_SUCCESS;
    TCHAR szNewPathName[MAX_PATH], szResult[MAX_PATH];
    ::lstrcpy (szResult, szArchiveFPN);

    if (_taccess (szDestFolderPath, 0) == -1 && MakeDir (szDestFolderPath) == -1)
        return FL_EACCESS;

    ::lstrcpy (szNewPathName, szDestFolderPath);
    ::PathAddBackslash (szNewPathName);
    ::lstrcat (szNewPathName, ::PathFindFileName (szResult));

    if (_taccess (szArchiveFPN, 0) == -1)
        return ret;

    switch (GetArchiveType(szArchiveFPN))
    {
    case ARC_ZIP:
        {
#ifdef _UNICODE
            char szFileNameInArchiveA[MAX_PATH];
            ::WideCharToMultiByte (CP_ACP, 0, szFileNameInArchive, -1, szFileNameInArchiveA,
                MAX_PATH, NULL, NULL);
            ret = ExtractFileInZip (szArchiveFPN, szFileNameInArchiveA, szNewPathName);
#else
            ret = ExtractFileInZip (szArchiveFPN, szFileNameInArchive, szNewPathName);
#endif
        }
        break;

    case ARC_RAR:
    default:
        ret = FL_ENOTSUPT;
    }

    if (ret == FL_SUCCESS && bRenFile) {
        ::lstrcpy (szResult, szNewPathName);
        ::lstrcat (szResult, _T(".vir"));
        ::MoveFileEx (szNewPathName, szResult, MOVEFILE_REPLACE_EXISTING);
    }
    else if (ret == FL_EWRITE && bMoveRenLockedFiles) {
        // TODO: The file might be locked, try move/rename it at next system restart.
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//  ZIP SUPPORT
//


int CArchiveOp::ScanZipFile (LPCTSTR szZipFPN)
{
    unsigned int i;
    int ret = FL_CLEAN, zerr;
    unzFile uf;
    unz_global_info ugi;

    /* open archive */
    uf = unzOpen (szZipFPN);
    if (uf == NULL) {
        return FL_EARC;
    }

    /* get its global info */
    zerr = unzGetGlobalInfo (uf, &ugi);
    if (zerr != UNZ_OK) {
        unzClose (uf);
        return FL_EARC;
    }

    /* iterate each entry */
    for (i = 0; i < ugi.number_entry; ++i) {
        unz_file_info ufi;
        char szFileNameInZip[MAX_PATH];  /* filename in zip cannot unicode */
        char* pszFileNameInZipWOPath;    /* filename without path */
        char* p;
        _TCHAR szTempFileName[MAX_PATH];

        /* get current file info */
        zerr = unzGetCurrentFileInfo (uf, &ufi, szFileNameInZip, sizeof(szFileNameInZip),
            NULL, 0, NULL, 0);
        if (zerr != UNZ_OK) {
            ret = FL_EARC;
            break;
        }

        /* Get filename without path in pszFileNameInZipWOPath */
        p = pszFileNameInZipWOPath = szFileNameInZip;
        while (*p != '\0') {
            if (*p == '/')
                pszFileNameInZipWOPath = p + 1;
            ++p;
        }

        /* 
            Check if the szFileNameInZip is directory path or filename, we expect filename.
            From the previous iteration, if szFileNameInZip is a directory name,
            the last char will be '/' and *pszFileNameInZipWOPath will be '\0'.
        */
        if (*pszFileNameInZipWOPath != '\0') {
            FILE* fTemp;
            zerr = unzOpenCurrentFile (uf);
            if (zerr != UNZ_OK) {
                ret = FL_EARC;
                break;
            }

            /* Create temp file name */
            CreateTempFilePathName (szTempFileName);

            /* open temp file */
            fTemp = _tfopen (szTempFileName, _T("w+b"));
            if (fTemp == NULL) {
                unzCloseCurrentFile (uf);
                ret = FL_ETEMPF;
                break;
            }
            else {
                /* Extract a file in zip file to szTempFileName */
                do {
                    char buff[FILEBUFF];
                    zerr = unzReadCurrentFile (uf, buff, sizeof(buff));
                    if (zerr < 0) {
                        /* If failed, one of cause might be the file was encrypted */
                        ret = FL_EARC;
                        break;
                    }

                    if (zerr > 0) {
                        /* If success, write extracted file */
                        if (fwrite (buff, 1, zerr, fTemp) != zerr) {
                            ret = FL_EIO;
                            break;
                        }
                    }
                } while (zerr > 0);

                fclose (fTemp);

                /* Scan extracted file */
                if (zerr == UNZ_OK) {
                    //FileInArchiveInfo fia_info;
                    //TCHAR szCombinedName[MAX_PATH];
                    //BuildCombinedArcFileName (szCombinedName, szZipFPN, szFileNameInZip);
                    //fia_info.nArchiveFileNameLen = ::lstrlen (szZipFPN);
                    //fia_info.nArchiveType = ARC_ZIP;
                    //fia_info.pszCombinedFileName = szCombinedName;
                    //ret = ScanFile (szTempFileName, &fia_info);
                    ret = Sca_ScanFile (szTempFileName, m_pFvdBlock);
                    if (ret == FL_VIRUS || ret == FL_SUSPECT) {
                        LPMWOBJ pMWObj = new MWOBJ;
                        pMWObj->dwType = MWT_ARCHIVE;
                        pMWObj->nScanResult = ret;
                    #ifdef _UNICODE
                        TCHAR szU[MAX_PATH];
                        ::MultiByteToWideChar (CP_ACP, 0, szFileNameInZip, -1, szU, MAX_PATH);
                        _tcscpy (pMWObj->szFileNameInArchive, szU);
                    #else
                        _tcscpy (pMWObj->szFileNameInArchive, szFileNameInZip);
                    #endif
                        _tcscpy (pMWObj->szFilePathName, szZipFPN);
                        m_MWObjPtrArray.Add (pMWObj);
                    }
                    ++m_uScanCount;
                }

                _tremove (szTempFileName);
            }

            unzCloseCurrentFile (uf);

            if (zerr != UNZ_OK)
                break;

        }   /* if (*pszFileNameInZipWOPath != '\0') */

        /* go to next filename in zip file */
        if ((i+1) < ugi.number_entry) {
            zerr = unzGoToNextFile (uf);
            if (zerr != UNZ_OK) {
                ret = FL_EARC;
                break;
            }
        }

        /* Process option dwMaxFileExtract */
        //--m_pScanOpt->archive_opt.dwMaxFileExtract;
        //if (m_pScanOpt->archive_opt.dwMaxFileExtract <= 0)
        //    break;
        
    }   /* END for (i = 0; i < ugi.number_entry; ++i) */

    unzClose (uf);
    return ret;
}

int CArchiveOp::DeleteFileInZip (LPCTSTR szZipFPN, const char* szDelFileNameInZip, 
                                 BOOL bDelPermanent)
{
    unsigned int i;
    int ret = FL_SUCCESS, zerr;
    zipFile zf;
    unzFile uf;
    unz_global_info ugi;
    TCHAR szTempZipFilePath[MAX_PATH];

    /* open zip archive for unzipping */
    uf = unzOpen (szZipFPN);
    if (uf == NULL)
        return FL_EOPEN;

    /* get its global info */
    zerr = unzGetGlobalInfo (uf, &ugi);
    if (zerr != UNZ_OK) {
        unzClose (uf);
        return FL_EARC;
    }

    /* create new temp zip archive */
    CreateTempFilePathName (szTempZipFilePath);
    zf = zipOpen (szTempZipFilePath, APPEND_STATUS_CREATE);
    if (zf == NULL) {
        unzClose (uf);
        return FL_ETEMPF;
    }

    /* iterate each entry and copy every entry into new temp zip */
    for (i = 0; i < ugi.number_entry; ++i) {
        unz_file_info ufi;

        char szFileNameInZip[MAX_PATH];  /* note: filename in zip cannot be unicode */
        char* pszFileNameInZipWOPath;    /* filename in zip without path */
        char* p;
        char buff[FILEBUFF];

        /* get current file info */
        zerr = unzGetCurrentFileInfo (uf, &ufi, szFileNameInZip, sizeof(szFileNameInZip),
            NULL, 0, NULL, 0);
        if (zerr != UNZ_OK) {
            ret = FL_EARC;
            break;
        }

        /* Get filename in zip without path in pszFileNameInZipWOPath */
        p = pszFileNameInZipWOPath = szFileNameInZip;
        while (*p != '\0') {
            if (*p == '/')
                pszFileNameInZipWOPath = p + 1;
            ++p;
        }

        /* Don't copy entry that will be deleted */
        if (_strcmpi (szDelFileNameInZip, szFileNameInZip) != 0) {
            /* 
                Check if the szFileNameInZip is directory path or filename, we expect filename.
                From the previous iteration, if szFileNameInZip is a directory name,
                the last char will be '/' and *pszFileNameInZipWOPath will be '\0'.
            */
            if (*pszFileNameInZipWOPath != '\0') {  // if file is filename
                zip_fileinfo zfi;

                /* Open unzip file */
                int iMethod, iLevel;
                zerr = unzOpenCurrentFile2 (uf, &iMethod, &iLevel, 1);
                if (zerr != UNZ_OK) {
                    ret = FL_EARC;
                    break;
                }

                /* Initialize zfi from ufi */
                zfi.dosDate = ufi.dosDate;
                zfi.external_fa = ufi.external_fa;
                zfi.internal_fa = ufi.internal_fa;
                memcpy (&zfi.tmz_date, &ufi.tmu_date, sizeof(zfi.tmz_date));

                /* Open temp zip file */
                zerr = zipOpenNewFileInZip2 (zf, szFileNameInZip, &zfi, 
                    NULL, 0, NULL, 0, NULL, iMethod, iLevel, 1);
                if (zerr != ZIP_OK) {
                    ret = FL_EARC;
                    break;
                }

                /* Extract raw data from unzip file to new temp zip file */
                do {
                    zerr = unzReadCurrentFile (uf, buff, sizeof(buff));
                    if (zerr < 0) {
                        /* If failed, one of cause might be the file was encrypted */
                        ret = FL_EARC;
                        break;
                    }
                    /* zerr contains the number of bytes read */
                    if (zerr > 0) {
                        /* If success, write raw file */
                       if (zipWriteInFileInZip (zf, buff, zerr) != ZIP_OK) {
                           ret = FL_EARC;
                           break;
                       }
                    }
                } while (zerr > 0);

                zipCloseFileInZipRaw (zf, ufi.uncompressed_size, ufi.crc);
                unzCloseCurrentFile (uf);

                if (zerr != ZIP_OK)
                    break;
            }
        }
        else {
            if (!bDelPermanent) {
                /* Open unzip file */
                zerr = unzOpenCurrentFile (uf);
                if (zerr != UNZ_OK) {
                    ret = FL_EARC;
                    break;
                }

                /* Create temp filename */
                TCHAR szExtractedFilePathName[MAX_PATH];
                ::GetTempPath (MAX_PATH, szExtractedFilePathName);
            #ifdef _UNICODE
                TCHAR szFileNameInZipW[MAX_PATH];
                ::MultiByteToWideChar (CP_ACP, 0, pszFileNameInZipWOPath, -1, 
                    szFileNameInZipW, MAX_PATH);
                ::lstrcat (szExtractedFilePathName, szFileNameInZipW);
            #else
                ::lstrcat (szExtractedFilePathName, pszFileNameInZipWOPath);
            #endif
                /* Create new file for writing */
                FILE* file = _tfopen (szExtractedFilePathName, TEXT("w+b"));
                if (file == NULL) {
                    ret = FL_ETEMPF;
                    break;
                }

                /* Extract malware file in zip to the new file */
                do {
                    zerr = unzReadCurrentFile (uf, buff, sizeof(buff));
                    if (zerr < 0) {
                        /* If failed, one of cause might be the file was encrypted */
                        ret = FL_EARC;
                        break;
                    }
                    /* zerr contains the number of bytes read */
                    if (zerr > 0) {
                        if (fwrite (buff, 1, zerr, file) != zerr) {
                            ret = FL_EIO;
                            break;
                        }
                    }
                } while (zerr > 0);

                fclose (file);
                unzCloseCurrentFile (uf);

                if (zerr != UNZ_OK)
                    break;

                // Delete it to recycle bin
                if (!DeleteFileToRecycleBin (szExtractedFilePathName)) {
                    ::DeleteFile (szExtractedFilePathName);
                    ret = FL_EWRITE;
                }
            }
        }

        /* go to next filename in zip file */
        if ((i+1) < ugi.number_entry) {
            zerr = unzGoToNextFile (uf);
            if (zerr != UNZ_OK) {
                ret = FL_EARC;
                break;
            }
        }
    }   /* END for */

    unzClose (uf);
    zipClose (zf, NULL);

    if (ret == FL_SUCCESS) {
        if (!::MoveFileEx (szTempZipFilePath, szZipFPN, MOVEFILE_REPLACE_EXISTING)) {
            ::DeleteFile (szTempZipFilePath);
            ret = FL_EWRITE;
        }
    }
    else
        ::DeleteFile (szTempZipFilePath);
    return ret;
}

int CArchiveOp::ExtractFileInZip (LPCTSTR szZipFPN, const char* szRenFileNameInZip, 
                                  LPCTSTR szDestFilePathName)
{
    unsigned int i;
    int ret = FL_SUCCESS, zerr;
    zipFile zf;
    unzFile uf;
    unz_global_info ugi;
    TCHAR szTempZipFilePath[MAX_PATH];

    /* open zip archive for unzipping */
    uf = unzOpen (szZipFPN);
    if (uf == NULL)
        return FL_EOPEN;

    /* get its global info */
    zerr = unzGetGlobalInfo (uf, &ugi);
    if (zerr != UNZ_OK) {
        unzClose (uf);
        return FL_EARC;
    }

    /* create new temp zip archive */
    CreateTempFilePathName (szTempZipFilePath);
    zf = zipOpen (szTempZipFilePath, APPEND_STATUS_CREATE);
    if (zf == NULL) {
        unzClose (uf);
        return FL_ETEMPF;
    }

    /* iterate each entry and copy every entry into new temp zip */
    for (i = 0; i < ugi.number_entry; ++i) {
        unz_file_info ufi;

        char szFileNameInZip[MAX_PATH];  /* note: filename in zip cannot be unicode */
        char* pszFileNameInZipWOPath;    /* filename in zip without path */
        char* p;
        char buff[FILEBUFF];

        /* get current file info */
        zerr = unzGetCurrentFileInfo (uf, &ufi, szFileNameInZip, sizeof(szFileNameInZip),
            NULL, 0, NULL, 0);
        if (zerr != UNZ_OK) {
            ret = FL_EARC;
            break;
        }

        /* Get filename in zip without path in pszFileNameInZipWOPath */
        p = pszFileNameInZipWOPath = szFileNameInZip;
        while (*p != '\0') {
            if (*p == '/')
                pszFileNameInZipWOPath = p + 1;
            ++p;
        }

        /* Don't copy the entry that will be deleted */
        if (_strcmpi (szRenFileNameInZip, szFileNameInZip) != 0) {
            /* 
                Check if the szFileNameInZip is directory path or filename, we expect filename.
                From the previous iteration, if szFileNameInZip is a directory name,
                the last char will be '/' and *pszFileNameInZipWOPath will be '\0'.
            */
            if (*pszFileNameInZipWOPath != '\0') {  // if file is filename
                zip_fileinfo zfi;

                /* Open unzip file */
                int iMethod, iLevel;
                zerr = unzOpenCurrentFile2 (uf, &iMethod, &iLevel, 1);
                if (zerr != UNZ_OK) {
                    ret = FL_EARC;
                    break;
                }

                /* Initialize zfi from ufi */
                zfi.dosDate = ufi.dosDate;
                zfi.external_fa = ufi.external_fa;
                zfi.internal_fa = ufi.internal_fa;
                memcpy (&zfi.tmz_date, &ufi.tmu_date, sizeof(zfi.tmz_date));

                /* Open temp zip file */
                zerr = zipOpenNewFileInZip2 (zf, szFileNameInZip, &zfi, 
                    NULL, 0, NULL, 0, NULL, iMethod, iLevel, 1);
                if (zerr != ZIP_OK) {
                    ret = FL_EARC;
                    break;
                }

                /* Extract raw data from unzip file to new temp zip file */
                do {
                    zerr = unzReadCurrentFile (uf, buff, sizeof(buff));
                    if (zerr < 0) {
                        /* If failed, one of cause might be the file was encrypted */
                        ret = FL_EARC;
                        break;
                    }
                    /* zerr contains the number of bytes read */
                    if (zerr > 0) {
                        /* If success, write raw file */
                       zerr = zipWriteInFileInZip (zf, buff, zerr);
                       if (zerr != ZIP_OK) {
                           ret = FL_EARC;
                           break;
                       }
                    }
                } while (zerr > 0);

                zipCloseFileInZipRaw (zf, ufi.uncompressed_size, ufi.crc);
                unzCloseCurrentFile (uf);

                if (zerr != ZIP_OK)
                    break;

            }
        }
        else {
            /* Open unzip file */
            zerr = unzOpenCurrentFile (uf);
            if (zerr != UNZ_OK) {
                ret = FL_EARC;
                break;
            }

            /* Create new file for writing */
            FILE* file = _tfopen (szDestFilePathName, TEXT("w+b"));
            if (file == NULL) {
                ret = FL_ETEMPF;
                break;
            }

            /* Extract malware file in zip to the new file */
            do {
                zerr = unzReadCurrentFile (uf, buff, sizeof(buff));
                if (zerr < 0) {
                    /* If failed, one of cause might be the file was encrypted */
                    ret = FL_EARC;
                    break;
                }
                /* zerr contains the number of bytes read */
                if (zerr > 0) {
                    if (fwrite (buff, 1, zerr, file) != zerr) {
                        ret = FL_EIO;
                        break;
                    }
                }
            } while (zerr > 0);

            fclose (file);
            unzCloseCurrentFile (uf);

            if (zerr != UNZ_OK)
                break;
        }

        /* go to next filename in zip file */
        if ((i+1) < ugi.number_entry) {
            zerr = unzGoToNextFile (uf);
            if (zerr != UNZ_OK) {
                ret = FL_EARC;
                break;
            }
        }
    }   /* END for */

    unzClose (uf);
    zipClose (zf, NULL);

    if (ret == FL_SUCCESS) {
        if (!::MoveFileEx (szTempZipFilePath, szZipFPN, MOVEFILE_REPLACE_EXISTING)) {
            ::DeleteFile (szTempZipFilePath);
            ret = FL_EWRITE;
        }
    }
    else
        ::DeleteFile (szTempZipFilePath);
    return ret;
}