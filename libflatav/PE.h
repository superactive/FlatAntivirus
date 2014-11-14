/****************************************************************************\
*
*	PE Processing Functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#ifndef PE_H_FLATAV_
#define PE_H_FLATAV_


#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* STRUCTS */

#define PE_TYPE_NONE  0
#define PE_TYPE_32    1
#define PE_TYPE_64    2

#define PE_FILE_HEADER_SIZE     0x14    /* 20  */
#define PE_HEADER32_SIZE        0xf8    /* 248 */
#define PE_HEADER64_SIZE        0x108   /* 264 */

#define IMAGE_FILE_DLL          0x2000 

typedef struct pe_header32_t
{
    unsigned short wMachine;
    unsigned short wNumberOfSections;
    unsigned short wCharacteristics;
    unsigned short wMagic;
    unsigned int dwAddressOfEntryPoint;
    unsigned int dwSectionAlignment;
    unsigned int dwFileAlignment;
    unsigned int dwSizeOfImage;
    unsigned int dwSizeOfHeaders;
    unsigned int dwImageBase;
}
PE_HEADER32, *LPPE_HEADER32;

typedef struct pe_header64_t
{
    unsigned short wMachine;
    unsigned short wNumberOfSections;
    unsigned short wCharacteristics;
    unsigned short wMagic;
    unsigned int dwAddressOfEntryPoint;
    unsigned int dwSectionAlignment;
    unsigned int dwFileAlignment;
    unsigned int dwSizeOfImage;
    unsigned int dwSizeOfHeaders;
    unsigned long long ddImageBase;
}
PE_HEADER64, *LPPE_HEADER64;

typedef union
{
    PE_HEADER32 peHdr32;
    PE_HEADER64 peHdr64;
}
PE_HEADER, *LPPE_HEADER;


#define PE_SECTION_HEADER_SIZE  0x28    // 40

typedef struct pe_section_header_t 
{
    unsigned char  bName[8];
    unsigned int   dwVirtualSize;
    unsigned int   dwVirtualAddress;
    unsigned int   dwSizeOfRawData;
    unsigned int   dwPointerToRawData;
    unsigned int   dwCharacteristics;
} 
PE_SECTION_HEADER, *LPPE_SECTION_HEADER;

typedef struct pe_data_directory_t
{
    unsigned int    dwVirtualAddress;
    unsigned int    dwSize;
}
PE_DATA_DIRECTORY, *LPPE_DATA_DIRECTORY;

#define PE_DIRECTORY_ENTRY_EXPORT          0    // 78h Export Directory *
#define PE_DIRECTORY_ENTRY_IMPORT          1    // 80h Import Directory *
#define PE_DIRECTORY_ENTRY_RESOURCE        2    // 88h Resource Directory
#define PE_DIRECTORY_ENTRY_EXCEPTION       3    // 90h Exception Directory
#define PE_DIRECTORY_ENTRY_SECURITY        4    // 98h Security Directory
#define PE_DIRECTORY_ENTRY_BASERELOC       5    // A0h Base Relocation Table
#define PE_DIRECTORY_ENTRY_DEBUG           6    // A8h Debug Directory
#define PE_DIRECTORY_ENTRY_ARCHITECTURE    7    // B0h Architecture Specific Data
#define PE_DIRECTORY_ENTRY_GLOBALPTR       8    // B8h RVA of GP
#define PE_DIRECTORY_ENTRY_TLS             9    // C0h TLS Directory
#define PE_DIRECTORY_ENTRY_LOAD_CONFIG    10    // C8h Load Configuration Directory
#define PE_DIRECTORY_ENTRY_BOUND_IMPORT   11    // D0h Bound Import Directory in headers
#define PE_DIRECTORY_ENTRY_IAT            12    // D8h Import Address Table
#define PE_DIRECTORY_ENTRY_DELAY_IMPORT   13    // E0h Delay Load Import Descriptors
#define PE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14    // E8h COM Runtime descriptor

/* Resource Directory structs */
typedef struct pe_resource_directory_t
{
    unsigned int      dwCharacteristics;        // 00h
    unsigned int      dwTimeDateStamp;          // 04h
    unsigned short    wMajorVersion;            // 08h
    unsigned short    wMinorVersion;            // 0Ah
    unsigned short    wNumberOfNamedEntries;    // 0Ch Number of Array of IMAGE_RESOURCE_DIRECTORY_ENTRY that using Name *
    unsigned short    wNumberOfIdEntries;       // 0Eh Number of Array of IMAGE_RESOURCE_DIRECTORY_ENTRY that using Id *
}
PE_RESOURCE_DIRECTORY, *LPPE_RESOURCE_DIRECTORY;

typedef struct pe_resource_directory_entry_t
{
    union {
        struct {
            unsigned int dwNameOffset:31;     // If hi-bit is clear, it's using Id (number)
            unsigned int bNameIsString:1;    // If hi-bit is set, it's using Name (string)
        };
        unsigned int    dwName; // 31 bits is an offset from beginning of resource section to IMAGE_RESOURCE_DIR_STRING_U
        unsigned short  wId;    // Id number
    };
    union {
        unsigned int   dwOffsetToData;
        struct {
            unsigned int   dwOffsetToDirectory:31;
            unsigned int   bDataIsDirectory:1;
        };
    };
} 
PE_RESOURCE_DIRECTORY_ENTRY, *LPPE_RESOURCE_DIRECTORY_ENTRY;

typedef struct
{
    unsigned int   dwOffsetToData;    // RVA to data (RVA to resource section + offset from beginning of resource section to data)
    unsigned int   dwSize;            // Size of data
    unsigned int   dwCodePage;        // Usually unicode code page
    unsigned int   dwReserved;
}
PE_RESOURCE_DATA_ENTRY, *LPPE_RESOURCE_DATA_ENTRY;

/****************************************************************************/
/* EXPORTED FUNCTIONS */

int PE_GetType (int handle);
int PE_ParseHeader (int handle, LPPE_HEADER lpPEHdr);
int PE_ParseSectionHeader (int handle, int iSectionIdx, LPPE_SECTION_HEADER lpPESectionHdr);
int PE_ParseDataDirectory (int handle, int iEntryIdx, LPPE_DATA_DIRECTORY lpPEDataDir);
//int PE_GetNumberOfSections (int handle);
//unsigned int PE_GetEntryPointAddr (int handle);
unsigned int PE_GetAllSectionRawDataSize (int handle);
unsigned int PE_RVAToOffset (int handle, unsigned int dwRVA, LPPE_HEADER lpPeHdr);

#ifdef __cplusplus
}
#endif

#endif 