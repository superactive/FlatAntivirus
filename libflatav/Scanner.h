/****************************************************************************\
*
*	FlatAV Scanner Engine
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#ifndef SCANNER_H_54878
#define SCANNER_H_54878

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef struct sca_pe_header_t
{
    uint32_t dwPEOffset;
    uint16_t wMachine;
    uint16_t wNumberOfSections;
    uint32_t dwAddressOfEntryPoint;
    uint32_t dwImageBase;
    uint32_t dwSectionAlignment;
    uint32_t dwFileAlignment;
    uint32_t dwSizeOfImage;
    uint32_t dwSizeOfHeaders;
}
SCA_PE_HEADER, *LPSCA_PE_HEADER;

typedef struct sca_section_header_t
{
    char szName[8+1];
    uint32_t dwVirtualSize;
    uint32_t dwVirtualAddress;
    uint32_t dwSizeOfRawData;
    uint32_t dwPointerToRawData;
    uint32_t dwCharacteristics;
}
SCA_SECTION_HEADER, *LPSCA_SECTION_HEADER;
*/

typedef struct sca_option_t
{
    struct archive_opt_t
    {
        int fScanArchive;       /* 1 = scan archive, 0 = no scan archive */
        int dwMaxFileExtract;   /* Max files to be extracted for scanning */
        int dwMaxSubArcExtract; /* Max sub-archive to be extracted for scanning */
        int dwMaxScanArcSize;   /* Max archive size for scanning */
    } 
    archive_opt;
}
SCA_OPT, *LPSCA_OPT;



#ifdef __cplusplus
}
#endif

#endif