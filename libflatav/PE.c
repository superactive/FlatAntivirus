/****************************************************************************\
*
*	PE Processing Functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include <stdio.h>
#include <io.h>
//#include <fcntl.h>  /* for mode constant in _tsopen */
//#include <share.h>  /* for share mode consant in _tsopen */
//#include <errno.h>  /* for global variable errno */
//#include <excpt.h>  /* for EXCEPTION_EXECUTE_HANDLER */
#include "PE.h"
#include "macro.h"

#define IMAGE_DOS_SIGNATURE     0x5a4d  /* MZ */
#define IMAGE_DOS_SIGNATURE_R   0x4d5a  /* ZM */
#define IMAGE_NT_SIGNATURE      0x4550  /* PE */
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b

/* lower and upper bondary alignment (size vs offset) */
//#define PEALIGN(o,a) (((a))?(((o)/(a))*(a)):(o))
//#define PESALIGN(o,a) (((a))?(((o)/(a)+((o)%(a)!=0))*(a)):(o))

#define PE_DATA_DIRECTORIES_SIZE    0x80

int PE_GetType (int handle)
{
    char buffer[4];
    unsigned int dwPEOffset;
    long lOldFilePos;
    /* Save current file position */
    lOldFilePos = _tell (handle);

    /* Check existence of "MZ" signature */
    _read (handle, buffer, 2);
    if (PTR(unsigned short, buffer[0]) != IMAGE_DOS_SIGNATURE)
        return 0;

    /* Goto IMAGE_DOS_HEADER.e_lfanew & read its value */
    _lseek (handle, 0x3c, SEEK_SET);
    if (_read (handle, &dwPEOffset, 4) == 0)
        return -1;

    /* Goto IMAGE_NT_HEADERS */
    _lseek (handle, dwPEOffset, SEEK_SET);

    /* Check existence of "PE" signature */
    if (_read (handle, buffer, 2) == 0)
        return -1;
    if (PTR(unsigned short, buffer[0]) != IMAGE_NT_SIGNATURE)
        return 0;

    /* Check existence of PE32 Signature (0x020b) */
    _lseek (handle, dwPEOffset+0x18, SEEK_SET);
    if (_read (handle, buffer, 2) == 0)
        return -1;
    if (PTR(unsigned short, buffer[0]) == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return PE_TYPE_32;   /* File is PE32 */
    if (PTR(unsigned short, buffer[0]) == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        return PE_TYPE_64;   /* File is PE64 */

    _lseek (handle, lOldFilePos, SEEK_SET);
    return 0;
}

int PE_ParseHeader (int handle, LPPE_HEADER lpPEHdr)
{
    unsigned int dwOffset;
    long lOldFilePos;
    /* Save current file position */
    lOldFilePos = _tell (handle);

    /* Goto IMAGE_DOS_HEADER.e_lfanew & read its value */
    _lseek (handle, 0x3c, SEEK_SET);
    if (_read (handle, &dwOffset, 4) == 0)
        return -1;

    /* Goto IMAGE_NT_HEADERS */
    _lseek (handle, dwOffset, SEEK_SET);

    /* Read field Machine */
    _lseek (handle, 0x4, SEEK_CUR);
    if (_read (handle, &lpPEHdr->peHdr32.wMachine, sizeof(lpPEHdr->peHdr32.wMachine)) == 0)
        return -1;

    /* Read field NumberOfSections */
    if (_read (handle, &lpPEHdr->peHdr32.wNumberOfSections, sizeof(lpPEHdr->peHdr32.wNumberOfSections)) == 0)
        return -1;

    /* Read field Charactersitics */
    _lseek (handle, 0xe, SEEK_CUR);
    if (_read (handle, &lpPEHdr->peHdr32.wCharacteristics, sizeof(lpPEHdr->peHdr32.wCharacteristics)) == 0)
        return -1;

    /* Read field Magic */
    if (_read (handle, &lpPEHdr->peHdr32.wMagic, sizeof(lpPEHdr->peHdr32.wMagic)) == 0)
        return -1;

    /* Read field AddressOfEntryPoint */
    _lseek (handle, 0xe, SEEK_CUR);
    if (_read (handle, &lpPEHdr->peHdr32.dwAddressOfEntryPoint, sizeof(lpPEHdr->peHdr32.dwAddressOfEntryPoint)) == 0)
        return -1;

    /* Read field ImageBase */
    _lseek (handle, 0x8, SEEK_CUR);
    if (lpPEHdr->peHdr32.wMagic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        if (_read (handle, &lpPEHdr->peHdr32.dwImageBase, sizeof(lpPEHdr->peHdr32.dwImageBase)) == 0)
            return -1;

        /* Read field SectionAlignment */
        if (_read (handle, &lpPEHdr->peHdr32.dwSectionAlignment, sizeof(lpPEHdr->peHdr32.dwSectionAlignment)) == 0)
            return -1;

        /* Read field FileAlignment */
        if (_read (handle, &lpPEHdr->peHdr32.dwFileAlignment, sizeof(lpPEHdr->peHdr32.dwFileAlignment)) == 0)
            return -1;

        /* Read field SizeOfImage */
        _lseek (handle, 0x10, SEEK_CUR);
        if (_read (handle, &lpPEHdr->peHdr32.dwSizeOfImage, sizeof(lpPEHdr->peHdr32.dwSizeOfImage)) == 0)
            return -1;

        /* Read field SizeOfHeaders */
        if (_read (handle, &lpPEHdr->peHdr32.dwSizeOfHeaders, sizeof(lpPEHdr->peHdr32.dwSizeOfHeaders)) == 0)
            return -1;
    }
    else {
        if (_read (handle, &lpPEHdr->peHdr64.ddImageBase, sizeof(lpPEHdr->peHdr64.ddImageBase)) == 0)
            return -1;

        /* Read field SectionAlignment */
        if (_read (handle, &lpPEHdr->peHdr64.dwSectionAlignment, sizeof(lpPEHdr->peHdr64.dwSectionAlignment)) == 0)
            return -1;

        /* Read field FileAlignment */
        if (_read (handle, &lpPEHdr->peHdr64.dwFileAlignment, sizeof(lpPEHdr->peHdr64.dwFileAlignment)) == 0)
            return -1;

        /* Read field SizeOfImage */
        _lseek (handle, 0x10, SEEK_CUR);
        if (_read (handle, &lpPEHdr->peHdr64.dwSizeOfImage, sizeof(lpPEHdr->peHdr64.dwSizeOfImage)) == 0)
            return -1;

        /* Read field SizeOfHeaders */
        if (_read (handle, &lpPEHdr->peHdr64.dwSizeOfHeaders, sizeof(lpPEHdr->peHdr64.dwSizeOfHeaders)) == 0)
            return -1;
    }

    _lseek (handle, lOldFilePos, SEEK_SET);
    return 0;
}

int PE_ParseSectionHeader (int handle, int iSectionIdx, LPPE_SECTION_HEADER lpPESectionHdr)
{
    unsigned int dwOffset;
    unsigned short wField;  /* Hold NumberOfSections & SizeOfOptionalHeader value */
    long lOldFilePos;
    /* Save current file position */
    lOldFilePos = _tell (handle);

    /* Goto IMAGE_DOS_HEADER.e_lfanew & read its value */
    _lseek (handle, 0x3c, SEEK_SET);
    if (_read (handle, &dwOffset, 4) == 0)
        return -1;

    /* Check if iSectionIdx is valid or not by comparing with NumberOfSections */
    /*_lseek (handle, dwOffset + 0x6, SEEK_SET);
    if (_read (handle, &wField, 2) == 0)
        return -1;
    if (iSectionIdx >= wField)
        return -1;
    */

    /* Read IMAGE_FILE_HEADER.SizeOfOptionalHeader value */
    /*_lseek (handle, 0xc, SEEK_CUR);*/
    _lseek (handle, dwOffset + 0x14, SEEK_SET);
    if (_read (handle, &wField, 2) == 0)
        return -1;

    /* Goto IMAGE_SECTION_HEADER iSectionIdx'th and read its 24 bytes value*/
    _lseek (handle, 2 + wField + iSectionIdx * PE_SECTION_HEADER_SIZE, SEEK_CUR);
    if (_read (handle, lpPESectionHdr, 24) == 0)
        return -1;

    /* Goto IMAGE_SECTION_HEADER.Characteristics and read its value */
    _lseek (handle, 12, SEEK_CUR);
    if (_read (handle, &lpPESectionHdr->dwCharacteristics, 4) == 0)
        return -1;

    _lseek (handle, lOldFilePos, SEEK_SET);
    return 0;
}

int PE_ParseDataDirectory (int handle, int iEntryIdx, LPPE_DATA_DIRECTORY lpPEDataDir)
{
    unsigned int dwOffset;
    unsigned short wSizeOfOptionalHeader;  /* Hold NumberOfSections & SizeOfOptionalHeader value */
    long lOldFilePos;
    /* Save current file position */
    lOldFilePos = _tell (handle);

    /* Goto IMAGE_DOS_HEADER.e_lfanew & read its value */
    _lseek (handle, 0x3c, SEEK_SET);
    if (_read (handle, &dwOffset, 4) == 0)
        return -1;

    /* Read IMAGE_FILE_HEADER.SizeOfOptionalHeader value */
    _lseek (handle, dwOffset + 0x14, SEEK_SET);
    if (_read (handle, &wSizeOfOptionalHeader, 2) == 0)
        return -1;

    /* Goto IMAGE_DATA_DIRECTORY iEntryIdx'th and read its value*/
    _lseek (handle, 2 + wSizeOfOptionalHeader - PE_DATA_DIRECTORIES_SIZE + iEntryIdx * sizeof(PE_DATA_DIRECTORY), SEEK_CUR);
    if (_read (handle, lpPEDataDir, sizeof(PE_DATA_DIRECTORY)) == 0)
        return -1;

    _lseek (handle, lOldFilePos, SEEK_SET);
    return 0;
}

unsigned int PE_GetEntryPointAddr (int handle)
{
    unsigned int dwOffset;
    unsigned short dwEntryPointAddr = 0;
    long lOldFilePos;
    /* Save current file position */
    lOldFilePos = _tell (handle);

    /* Goto IMAGE_DOS_HEADER.e_lfanew & read its value */
    _lseek (handle, 0x3c, SEEK_SET);
    if (_read (handle, &dwOffset, 4) == 0)
        return -1;

    /* Read IMAGE_FILE_HEADER.NumberOfSections value */
    _lseek (handle, dwOffset + 0x28, SEEK_SET);
    if (_read (handle, &dwEntryPointAddr, 4) == 0)
        return -1;

    _lseek (handle, lOldFilePos, SEEK_SET);
    return dwEntryPointAddr;
}

int PE_GetNumberOfSections (int handle)
{
    unsigned int dwOffset;
    unsigned short wNumberOfSections = 0;
    long lOldFilePos;
    /* Save current file position */
    lOldFilePos = _tell (handle);

    /* Goto IMAGE_DOS_HEADER.e_lfanew & read its value */
    _lseek (handle, 0x3c, SEEK_SET);
    if (_read (handle, &dwOffset, 4) == 0)
        return -1;

    /* Read IMAGE_FILE_HEADER.NumberOfSections value */
    _lseek (handle, dwOffset + 0x6, SEEK_SET);
    if (_read (handle, &wNumberOfSections, 2) == 0)
        return -1;

    _lseek (handle, lOldFilePos, SEEK_SET);
    return wNumberOfSections;
}

unsigned int PE_GetAllSectionRawDataSize (int handle)
{
    unsigned int dwOffset;
    unsigned short wNumberOfSections, wSizeOfOptionalHeader;
    unsigned int dwSize, dwTotalSize = 0;
    int i;
    long lOldFilePos;
    /* Save current file position */
    lOldFilePos = _tell (handle);

    /* Goto IMAGE_DOS_HEADER.e_lfanew & read its value */
    _lseek (handle, 0x3c, SEEK_SET);
    if (_read (handle, &dwOffset, 4) == 0)
        return -1;

    /* Read IMAGE_FILE_HEADER.NumberOfSections value */
    _lseek (handle, dwOffset + 0x6, SEEK_SET);
    if (_read (handle, &wNumberOfSections, 2) == 0)
        return -1;

    /* Read IMAGE_FILE_HEADER.SizeOfOptionalHeader value */
    _lseek (handle, 0xc, SEEK_CUR);
    if (_read (handle, &wSizeOfOptionalHeader, 2) == 0)
        return -1;

    /* Goto IMAGE_SECTION_HEADER and save its offset to dwOffset */
    dwOffset = _lseek (handle, 2 + wSizeOfOptionalHeader, SEEK_CUR);

    /* Get all section raw data size */
    for (i = 0; i < wNumberOfSections; ++i) {        
        _lseek (handle, dwOffset + i * PE_SECTION_HEADER_SIZE + 0x10, SEEK_SET);
        if (_read (handle, &dwSize, 4) == 0)
            return -1;
        dwTotalSize += dwSize;
    }

    _lseek (handle, lOldFilePos, SEEK_SET);
    return dwTotalSize;
}

unsigned int PE_RVAToOffset (int handle, unsigned int dwRVA, LPPE_HEADER lpPeHdr)
{
	PE_SECTION_HEADER peSectionHdr;
	int i;
    for (i = 0; i < lpPeHdr->peHdr32.wNumberOfSections; ++i) {
        if (PE_ParseSectionHeader (handle, i, &peSectionHdr) == -1)
            return -1;
        if (dwRVA < 
            (peSectionHdr.dwVirtualAddress + ALIGN(peSectionHdr.dwVirtualSize, 
            lpPeHdr->peHdr32.dwSectionAlignment))) 
        {
            return peSectionHdr.dwPointerToRawData + (dwRVA - peSectionHdr.dwVirtualAddress);
        }
    }
	return 0;
}

