/****************************************************************************\
*
*	For processing archive files
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#pragma once

#define ZIP_SIGNATURE   0x04034b50
#define GZIP_SIGNATURE  0x8b1f
static const unsigned char SEVENZIP_SIGNATURE[6] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};

/* GetArchiveType return value */
#define ARC_NONE        0
#define ARC_ZIP         1
#define ARC_GZIP        2
#define ARC_BZIP2       3
#define ARC_TAR         4
#define ARC_RAR         5
#define ARC_CAB         6
#define ARC_ARJ         7
#define ARC_Z           8
#define ARC_CPIO        9
#define ARC_RPM         10
#define ARC_DEB         11
#define ARC_LZH         12
#define ARC_SPLIT       13
#define ARC_CHM         14
#define ARC_ISO         15
#define ARC_COMPOUND    16
#define ARC_WIM         17
#define ARC_NSIS        18
#define ARC_ACE         19
#define ARC_7Z          20

class CArchiveOp
{
public:
    CArchiveOp(LPFVD_BLOCK pFvdBlock)
    {
        m_pFvdBlock = pFvdBlock;
        m_uScanCount = 0;
    }
    static BOOL IsArchive (LPCTSTR szFilePathName);
    int GetArchiveType (LPCTSTR szFilePathName);
    int ScanArchiveFile (LPCTSTR szFilePathName);
    int DeleteFileInArchive (LPCTSTR szArchiveFPN, BOOL bDelPermanent, BOOL bDelLockedFiles, 
        LPCTSTR szFileNameInArchive);
    int MoveRenameFileInArchive (LPCTSTR szArchiveFPN, LPCTSTR szDestFolderPath, 
        BOOL bRenFile, BOOL bMoveRenLockedFiles, LPCTSTR szFileNameInArchive);

    int GetScannedMWCount() { return (int)m_MWObjPtrArray.GetSize(); }
    LPMWOBJ GetScannedMW(int i) { return m_MWObjPtrArray.GetAt(i); }
    UINT GetScanCount() { return m_uScanCount; }
protected:
    LPFVD_BLOCK m_pFvdBlock;
    CMWObjPtrArray m_MWObjPtrArray;
    UINT m_uScanCount;

    int ScanZipFile (LPCTSTR szZipFPN);
    int DeleteFileInZip (LPCTSTR szZipFPN, const char* szDelFileNameInZip, BOOL bDelPermanent);
    int ExtractFileInZip (LPCTSTR szZipFPN, const char* szRenFileNameInZip, LPCTSTR szDestFilePathName);
};