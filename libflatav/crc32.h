#ifndef CRC32_H_SDIWU
#define CRC32_H_SDIWU

#ifdef __cplusplus
extern "C" {
#endif

void FreeCrc32Table ();

void InitCrc32Table_Asm ();
unsigned long Crc32_Asm (const unsigned char* buffer, size_t len, unsigned long crc32);

void InitCrc32Table ();
unsigned long Crc32 (const unsigned char* buffer, size_t len, unsigned long crc32);

#ifdef __cplusplus
}
#endif

#endif	/*CRC32_H_SDIWU*/
