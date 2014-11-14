#ifndef CRC32FILE_H_17057
#define CRC32FILE_H_17057

//#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

//int Crc32File_Path (_TCHAR* szFilePath, unsigned long* dwCrc32);
int Crc32File_Handle (int handle, size_t len, unsigned long* dwCrc32);

#ifdef __cplusplus
}
#endif

#endif
