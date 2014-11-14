//#include <windows.h>
//#include <tchar.h>
#include <io.h>
//#include <fcntl.h>
//#include <share.h>
/*#include <errno.h>*/
#include "crc32.h"

#define BUFFER_SIZE    4096
/*
int Crc32File_Path (_TCHAR* szFilePath, unsigned long* dwCrc32)
{
    int handle = _tsopen (szFilePath,_O_RDONLY | _O_BINARY, _SH_DENYWR);
    if (handle != -1) {
        unsigned char buffer[BUFFER_SIZE];
        size_t dwBytesRead;
        *dwCrc32 = 0;
        do {
            dwBytesRead = _read (handle, buffer, sizeof(buffer));
            if (dwBytesRead == 0)
                break;
            if (dwBytesRead < 0)
                return -1;
            *dwCrc32 = Crc32_Asm (buffer, dwBytesRead, *dwCrc32);
        } while (1);
        _close (handle);
    }
    else
        return -1;
    return 0;
}
*/
/****************************************************************************
*
*   FUNCTION:   Crc32File_Handle
*
*   PURPOSE:    Calculate CRC32 from current file position.
*
*   PARAMS:
*               handle      - File handle.
*               len         - Bytes length to be calculated.
*               crc32       - Output CRC32 variable.
*
*   RETURNS:    int         - error code (0 = success, -1 = fail).
*
*   NOTE:       The operation affects file position.
*
\****************************************************************************/
int Crc32File_Handle (int handle, size_t len, unsigned long* dwCrc32)
{
    unsigned char buffer[BUFFER_SIZE];
    size_t dwBytesRead;
    *dwCrc32 = 0;

    do {
        dwBytesRead = _read (handle, buffer, sizeof(buffer));
        if (dwBytesRead == 0)
            break;
        if (dwBytesRead == -1)
            return -1;
        *dwCrc32 = Crc32_Asm (buffer, dwBytesRead, *dwCrc32);
        len -= dwBytesRead;
    } while (len > 0);

    return 0;
}


	/*
    unsigned int dwErrCode = 0;
    HANDLE hFile;

	hFile = CreateFile (
		szFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | 
			FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
			BOOL bSuccess;
			BYTE buffer[BUFFER_SIZE];
			DWORD dwBytesRead;

			*dwCrc32 = 0;
			do {
				bSuccess = ReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead, NULL);
				*dwCrc32 = CRC32_asm (buffer, dwBytesRead, *dwCrc32);
			} while (bSuccess && dwBytesRead > 0);
			CloseHandle (hFile);
	}
	else
		dwErrCode = GetLastError ();

	return dwErrCode;*/