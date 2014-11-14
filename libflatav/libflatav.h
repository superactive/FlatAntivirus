// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBFLATAV_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBFLATAV_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBFLATAV_EXPORTS
#define LIBFLATAV_API __declspec(dllexport)
#else
#define LIBFLATAV_API __declspec(dllimport)
#endif

//#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "fvd.h"
#include "scanner.h"

/* Fvd.h */
/*extern int Fvd_BuildNewDb (_TCHAR* szCDBFilePath, _TCHAR* szPDBFilePath, 
                    _TCHAR* szSDBFilePath, _TCHAR* szWDBFilePath,
                    unsigned int dwDbMayorVersion, unsigned char bDbMinorVersion,
                    unsigned short wMinAvMayorVersion, unsigned char bMinAvMinorVersion,
                    _TCHAR* szFVDFilePath);

extern int Fvd_LoadDb (_TCHAR* szDbFilePath, void* lpFvdDb);
#define Fvd_UnloadDb(p) free (p)
extern void Fvd_ParseHeader (void* lpFvdDb, LPFVD_HEADER lpFvdHeader);*/

/*
extern void FreeCrc32Table ();
extern void InitCrc32Table_asm ();
extern unsigned long Crc32_asm (const unsigned char* buffer, size_t len, unsigned long crc32);
extern void InitCrc32Table ();
extern unsigned long Crc32 (const unsigned char* buffer, size_t len, unsigned long crc32);
extern unsigned Crc32File_Path (TCHAR* szFilePath, unsigned* dwCRC32);
*/

#ifdef __cplusplus
}
#endif



// This class is exported from the libflatav.dll
/*
class LIBFLATAV_API Clibflatav {
public:
	Clibflatav(void);
	// TODO: add your methods here.
};
*/
//extern LIBFLATAV_API int nlibflatav;

//LIBFLATAV_API int fnlibflatav(void);
