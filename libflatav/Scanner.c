/****************************************************************************\
*
*	FlatAV Scanner Engine
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>     /* for _taccess & _tsopen */
#include <fcntl.h>  /* for mode constant in _tsopen */
#include <share.h>  /* for share mode consant in _tsopen */
#include <ctype.h>
#include <string.h>	/* for memcmp */
#include "flatav-int.h"
#include "fvd.h"
#include "crc32.h"
#include "crc32file.h"
#include "scanner.h"
#include "PE.h"
#include "script.h"
#include "list.h"
#include "macro.h"

/* Internal structs */
typedef struct sca_sign_hexbyte_t
{
	unsigned int dwMatchType;
	unsigned int dwMatchBytes;  /* used only if dwMatchType is 3, 4 or 5 */
	unsigned char aHexSignBytes[512]; /* used only if dwMatchType is 0 */
	unsigned int dwHexSignBytesLen;
}
SCA_SIGN_HEXBYTE, *LPSCA_SIGN_HEXBYTE;

#define FOM_LAST_SECTION   'k'

/* dwMatchType value */
#define SCA_MATCH_STR           0
#define SCA_MATCH_ANY           1
#define SCA_MATCH_ANY_BYTE      2
#define SCA_MATCH_N_BYTE        3
#define SCA_MATCH_N_BYTE_LESS   4
#define SCA_MATCH_N_BYTE_MORE   5


typedef struct sca_rsrc_sign_t
{
	char* szMalwareName;
	unsigned char bMalwareType;
	unsigned int dwHexSignLen;
	unsigned int dwRelOffset;   /* Relative offset from bitmap image data */
	unsigned char sbHexSign[8];
}
SCA_RSRC_SIGN;

/* Resource signature for suspected malware (only for .exe file) */
static const SCA_RSRC_SIGN rsrcSign[] = {
	"Suspected.XP-Folder", 1, 8, 0x85a, { 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x1F, 0xF0, 0x00 },
	//"Suspected.Win2k-Folder", 1, 4, 0x896, {0x7F,0xFF,0xC0,0x00}
	//"Suspected.Win2k-Folder", 1, 4, 0x82e, {0xFF, 0xFF, 0xFF, 0xFF}
};


static int RsrcScan(int handle, LPPE_HEADER lpPeHdr, LPFVD_BLOCK lpFvdBlock)
{
	PE_DATA_DIRECTORY peDataDir;
	PE_RESOURCE_DIRECTORY peResDir;

	unsigned int dwResOffset;
	char* buffer;
	int i, j;

	/* Mark PE file with some suspicious icon as malware suspected */
	/* Get resource data directory */
	if (PE_ParseDataDirectory(handle, PE_DIRECTORY_ENTRY_RESOURCE, &peDataDir) == -1)
		return FL_EPE;
	/* If size is 0, it means the file doesn't have resource directory */
	if (peDataDir.dwSize == 0)
		return FL_CLEAN;
	/* Convert resource data directory's RVA to file offset */
	dwResOffset = PE_RVAToOffset(handle, peDataDir.dwVirtualAddress, lpPeHdr);
	if (dwResOffset < 1)
		return FL_EPE;

	/* Goto root resource directory */
	_lseek(handle, dwResOffset, SEEK_SET);
	/* Read root resource directory */
	_read(handle, &peResDir, sizeof(peResDir));
	/* Iterate each resource-type resource directory entry */
	for (i = 0; i < (peResDir.wNumberOfIdEntries + peResDir.wNumberOfNamedEntries); ++i) {
		PE_RESOURCE_DIRECTORY_ENTRY peResDirEntry;
		/* Read resource-type resource directory entry */
		_read(handle, &peResDirEntry, sizeof(peResDirEntry));
		/* If resource directory entry's type is icon then keep go on */
		if (!peResDirEntry.bNameIsString && peResDirEntry.wId == 3) {
			long lOldFilePos;
			/* Go into resource-type resource directory */
			_lseek(handle, dwResOffset + peResDirEntry.dwOffsetToDirectory, SEEK_SET);
			/* Read resource-type resource directory */
			_read(handle, &peResDir, sizeof(peResDir));

			/* Save current file position */
			lOldFilePos = _tell(handle);
			/* Iterate each Id/Name resource directory */
			for (j = 0; j < (peResDir.wNumberOfIdEntries + peResDir.wNumberOfNamedEntries); ++j) {
				PE_RESOURCE_DATA_ENTRY peResDataEntry;

				/* Goto resource directory entry for each Id/Name found & read its value */
				_lseek(handle, lOldFilePos + j * sizeof(PE_RESOURCE_DIRECTORY_ENTRY), SEEK_SET);
				_read(handle, &peResDirEntry, sizeof(peResDirEntry));

				/* Go down into id/name resource directory */
				_lseek(handle, dwResOffset + peResDirEntry.dwOffsetToDirectory, SEEK_SET);

				/* Go directly to resource directory entry & read its value*/
				_lseek(handle, 0x10, SEEK_CUR);
				_read(handle, &peResDirEntry, sizeof(peResDirEntry));

				/* Goto resource data entry & read its value */
				_lseek(handle, dwResOffset + peResDirEntry.dwOffsetToData, SEEK_SET);
				_read(handle, &peResDataEntry, sizeof(peResDataEntry));

				/* Convert peResDataEntry.dwOffsetToData from RVA form to file offset form */
				peResDataEntry.dwOffsetToData = peResDataEntry.dwOffsetToData -
					peDataDir.dwVirtualAddress + dwResOffset;

				/* Check for each rsrcSign */
				for (i = 0; i < sizeof(rsrcSign) / sizeof(rsrcSign[0]); ++i) {
					_lseek(handle, peResDataEntry.dwOffsetToData + rsrcSign[i].dwRelOffset,
						SEEK_SET);
					buffer = (char*)malloc(rsrcSign[i].dwHexSignLen);
					if (buffer == NULL)
						return FL_ENOMEM;
					_read(handle, buffer, rsrcSign[i].dwHexSignLen);
					if (memcmp(buffer, rsrcSign[i].sbHexSign, rsrcSign[i].dwHexSignLen) == 0) {
						strcpy_s(lpFvdBlock->szMalwareName, _countof(lpFvdBlock->szMalwareName), rsrcSign[i].szMalwareName);
						lpFvdBlock->bMalwareType = rsrcSign[i].bMalwareType;
						free(buffer);
						return FL_SUSPECT;
					}
					free(buffer);
				}

			}
			break;
		}   /* END if (!peResDirEntry.bNameIsString && peResDirEntry.wId == 3) */
	}   /* END for (i = 0; i < (peResDir.wNumberOfIdEntries + ... */

	return FL_CLEAN;
}

int HexCharToInt(int ch)
{
	ch = toupper(ch);
	if (ch > 64 && ch < 71)
		return (ch - 55);
	if (ch > 47 && ch < 58)
		return (ch - 48);
	return -1;
}

int HextStrToBytes(const char* szHexStr, const char** endptr, unsigned char* aBytes)
{
	int i, err = 0;
	const char* p;
	for (p = szHexStr, i = 0; *p != '\0'; ++p, ++i) {
		int n;
		if (isxdigit(toascii(*p))) {
			n = HexCharToInt(*p) * 16;
			if (n == -1) {
				err = -1;
				break;
			}
			++p;
			if (isxdigit(toascii(*p))) {
				n += HexCharToInt(*p);
				if (n == -1) {
					err = -1;
					break;
				}
				aBytes[i] = n;
			}
			else {
				err = -1;
				break;
			}
		}
		else {
			err = -1;
			break;
		}
	}
	if (endptr != NULL)
		*endptr = p;

	return err;
}

static int ParseSignHexStr(const char* szHexStr, unsigned int dwLength, List* plstSignHexByte)
{
	unsigned int i;
	for (i = 0; i < dwLength; ++i) {
		LPSCA_SIGN_HEXBYTE lpSignHexByte = (LPSCA_SIGN_HEXBYTE)malloc(sizeof(SCA_SIGN_HEXBYTE));
		if (lpSignHexByte == NULL)
			return FL_ENOMEM;

		switch (szHexStr[i])
		{
		case '*':
			lpSignHexByte->dwMatchType = SCA_MATCH_ANY; break;

		case '?':
		{
			if (szHexStr[++i] == '?')
				lpSignHexByte->dwMatchType = SCA_MATCH_ANY_BYTE;
			else {
				free(lpSignHexByte);
				return -1;
			}
		}
		break;

		case '{':
		{
			char* endptr;
			if (szHexStr[++i] == '-') {
				lpSignHexByte->dwMatchType = SCA_MATCH_N_BYTE_LESS;
				++i;
			}
			else
				lpSignHexByte->dwMatchType = SCA_MATCH_N_BYTE;

			lpSignHexByte->dwMatchBytes = strtoul(&szHexStr[i], &endptr, 0);
			i += (unsigned int)(endptr - &szHexStr[i]);

			if (szHexStr[i] == '-') {
				lpSignHexByte->dwMatchType = SCA_MATCH_N_BYTE_MORE;
				++i;
			}

			if (szHexStr[i] != '}') {
				free(lpSignHexByte);
				return -1;
			}
		}
		break;

		default:
		{
			unsigned int len;
			char* endptr;
			HextStrToBytes(&szHexStr[i], &endptr, lpSignHexByte->aHexSignBytes);
			len = (unsigned int)(endptr - &szHexStr[i]);
			if (len == 0) {
				free(lpSignHexByte);
				return -1;
			}
			i += (len - 1); // - 1 cause next there is ++i
			lpSignHexByte->dwHexSignBytesLen = len / 2;
			lpSignHexByte->dwMatchType = SCA_MATCH_STR;
		}
		}
		List_PushBack(plstSignHexByte, lpSignHexByte);
	}

	return 0;
}

static int SDBScan(int handle, LPFVD_SDBREC lpSdbRec, int iFileType, LPPE_HEADER lpPeHdr, LPFVD_BLOCK lpFvdBlock)
{
	int ret = FL_CLEAN, iDisp = 0;
	PE_SECTION_HEADER peSectionHdr;
	char* buffer;
	unsigned int dwBuffSize = 0;
	unsigned int i;

	/* Seek to offset of szFileOffset told */
	switch (lpSdbRec->szFileOffset[0])
	{
	case FVD_FO_SECTION:
	{
		int iSectionIdx = 0;
		if (iFileType != FVD_TT_PE && iFileType != FVD_TT_PE64)
			return FL_EFVD;

		if (isxdigit(lpSdbRec->szFileOffset[1]) || lpSdbRec->szFileOffset[1] == FOM_LAST_SECTION) {
			if (isxdigit(lpSdbRec->szFileOffset[1])) {
				iSectionIdx = HexCharToInt(lpSdbRec->szFileOffset[1]);
				if (iSectionIdx >= lpPeHdr->peHdr32.wNumberOfSections)
					return FL_CLEAN;
			}
			else
				iSectionIdx = lpPeHdr->peHdr32.wNumberOfSections - 1;

			if (lpSdbRec->szFileOffset[2] == '+' || lpSdbRec->szFileOffset[2] == '-')
				iDisp = strtol(&lpSdbRec->szFileOffset[2], NULL, 16);

			if (PE_ParseSectionHeader(handle, iSectionIdx, &peSectionHdr) == -1)
				return FL_EPE;

			dwBuffSize = peSectionHdr.dwSizeOfRawData;
		}
		else if (lpSdbRec->szFileOffset[1] == 'x') {  /* for $x */
			if (PE_ParseSectionHeader(handle, 0, &peSectionHdr) == -1)
				return FL_EPE;
			dwBuffSize = PE_GetAllSectionRawDataSize(handle);
		}
		else
			return FL_EFVD;

		_lseek(handle, peSectionHdr.dwPointerToRawData + iDisp, SEEK_SET);
	}
	break;

	case FVD_FO_EP:
	{
		unsigned int dwEntryPointOffset = 0;
		if (iFileType != FVD_TT_PE && iFileType != FVD_TT_PE64)
			return FL_EFVD;

		/* Find which section the entry point resides and get entry point offset */
		for (i = 0; i < lpPeHdr->peHdr32.wNumberOfSections; ++i) {
			if (PE_ParseSectionHeader(handle, i, &peSectionHdr) == -1)
				return FL_EPE;
			if (lpPeHdr->peHdr32.dwAddressOfEntryPoint <
				(peSectionHdr.dwVirtualAddress + ALIGN(peSectionHdr.dwVirtualSize,
				lpPeHdr->peHdr32.dwSectionAlignment)))
			{
				dwEntryPointOffset = peSectionHdr.dwPointerToRawData +
					(lpPeHdr->peHdr32.dwAddressOfEntryPoint - peSectionHdr.dwVirtualAddress);
				break;
			}
		}
		if (dwEntryPointOffset < 1)
			return FL_EPE;

		//if (lpSdbRec->szFileOffset[1] == '+')
		//    iDisp = strtol (&lpSdbRec->szFileOffset[2], NULL, 16);
		if (lpSdbRec->szFileOffset[1] == '+' || lpSdbRec->szFileOffset[1] == '-')
			iDisp = strtol(&lpSdbRec->szFileOffset[1], NULL, 16);
		else
			return FL_EFVD;

		_lseek(handle, dwEntryPointOffset + iDisp, SEEK_SET);
		//dwBuffSize = peSectionHdr.dwSizeOfRawData;
		dwBuffSize = peSectionHdr.dwSizeOfRawData - (dwEntryPointOffset - peSectionHdr.dwPointerToRawData);
	}
	break;

	case FVD_FO_EOF:
	{
		if (lpSdbRec->szFileOffset[1] == '-')
			iDisp = strtol(&lpSdbRec->szFileOffset[2], NULL, 16);
		else
			return FL_EFVD;

		_lseek(handle, iDisp, SEEK_END);
		dwBuffSize = -iDisp;
	}
	break;

	case FVD_FO_ANY:
	{
		_lseek(handle, 0, SEEK_SET);
		dwBuffSize = _filelength(handle);
	}
	break;

	default: /* szFileOffset is a hex number */
	{
		iDisp = strtoul(lpSdbRec->szFileOffset, NULL, 16);
		_lseek(handle, iDisp, SEEK_SET);
		dwBuffSize = _filelength(handle) - iDisp;
	}
	}

	if (dwBuffSize > 0) {
		//unsigned int dwHexStrLen;
		List lstSignHexByte;
		Node* node;

		/* TODO: checking for large buffer size and give special treatment */

		/* Load file into memory buffer */
		buffer = (char*)malloc(dwBuffSize + 1);
		if (buffer == NULL)
			return FL_ENOMEM;
		_read(handle, buffer, dwBuffSize);

		if (buffer[0] == '\0') {
			free(buffer);
			return FL_CLEAN;
		}

		if (iFileType == FVD_TT_SCRIPT) {
			/* Normalize if the file is a script */
			Script_Normalize(buffer, dwBuffSize);
		}

		/* Parse Sign Hex String into list nodes */
		List_Init(&lstSignHexByte);
		if (ParseSignHexStr(lpSdbRec->szSignHexStr, lpSdbRec->wRecSize - FVD_MIN_SDBREC_SIZE, &lstSignHexByte) == -1) {
			List_Clear(&lstSignHexByte, 1);
			free(buffer);
			return FL_EFVD;
		}

		/* Check sign hex string */
		for (i = 0; i < dwBuffSize; ++i) {
			int nPass = 0;
			unsigned int j = i;
			bool_t fMatchNByteMore = 0, fMatchNByteLess = 0, fMatchAnyNumBytes = 0, bFound;
			for (node = lstSignHexByte.begin; node != NULL; node = node->next, ++nPass) {
				bFound = 0;
				if (PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchType == SCA_MATCH_STR) {
					if (fMatchNByteMore || fMatchAnyNumBytes) {
						/* Careful, don't go over dwBuffSize */
						//unsigned int uCmpLen = PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwHexSignBytesLen;
						//while ((j+uCmpLen) <= dwBuffSize) {
						while (j < dwBuffSize) {
							if (memcmp(&buffer[j],
								PTR(LPSCA_SIGN_HEXBYTE, node->data)->aHexSignBytes,
								PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwHexSignBytesLen) == 0)
							{
								j += PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwHexSignBytesLen;
								bFound = 1;
								break;
							}
							++j;
						}
						fMatchNByteMore = fMatchAnyNumBytes = 0;
						if (!bFound)
							break;
					}
					else if (fMatchNByteLess > 0) {
						unsigned int uLimit = j + fMatchNByteLess;
						//unsigned int x = PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwHexSignBytesLen;
						//_TCHAR* pstr = PTR(LPSCA_SIGN_HEXBYTE, node->data)->aHexSignBytes;
						while (j <= uLimit) {
							if (memcmp(&buffer[j],
								PTR(LPSCA_SIGN_HEXBYTE, node->data)->aHexSignBytes,
								PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwHexSignBytesLen) == 0)
							{
								j += PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwHexSignBytesLen;
								bFound = 1;
								break;
							}
							++j;
						}
						fMatchNByteLess = 0;
						//if (j == uLimit)
						if (!bFound)
							break;
					}
					else {	// if (fMatchNByteMore)
						unsigned int dwHexSignBytesLen = PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwHexSignBytesLen;
						if ((j + dwHexSignBytesLen) < dwBuffSize &&
							memcmp(&buffer[j],
							PTR(LPSCA_SIGN_HEXBYTE, node->data)->aHexSignBytes,
							dwHexSignBytesLen) == 0)
						{
							j += dwHexSignBytesLen;
						}
						else
							break;
					}
				}
				else if (PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchType == SCA_MATCH_ANY_BYTE) {
					++j;
				}
				else if (PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchType == SCA_MATCH_N_BYTE) {
					j += PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchBytes;
				}
				else if (PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchType == SCA_MATCH_N_BYTE_LESS) {
					fMatchNByteLess = PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchBytes;
				}
				else if (PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchType == SCA_MATCH_N_BYTE_MORE) {
					j += PTR(LPSCA_SIGN_HEXBYTE, node->data)->dwMatchBytes;
					fMatchNByteMore = 1;
				}
				else {// SCA_MATCH_ANY
					fMatchAnyNumBytes = 1;
				}

				if (j >= dwBuffSize)
					break;
			}   /* END for (node = lstSignHexByte.begin; ... */

			if (nPass == lstSignHexByte.count) {
				strcpy_s(lpFvdBlock->szMalwareName, _countof(lpFvdBlock->szMalwareName), lpSdbRec->szMalwareName);
				lpFvdBlock->bMalwareType = lpSdbRec->bMalwareType;
				ret = FL_VIRUS;
				break;
			}
		}   /* END for (i = 0; i < dwBuffSize; ++i) */
		List_Clear(&lstSignHexByte, 1);
	}   /* END if (dwBuffSize > 0) */
	else return FL_CLEAN;

	free(buffer);
	return ret;
}

static int XDBScan(int handle, LPFVD_XDBREC lpXdbRec, LPPE_HEADER lpPeHdr, LPFVD_BLOCK lpFvdBlock)
{
	int iDisp = 0;
	PE_SECTION_HEADER peSectionHdr;
	char* buffer;
	unsigned int dwBuffCRC32;

	/* Seek to offset of szFileOffset told */
	switch (lpXdbRec->szFileOffset[0])
	{
	case FVD_FO_SECTION:
	{
		int iSectionIdx;
		if (lpPeHdr == NULL)
			return FL_EFVD;

		if (isxdigit(lpXdbRec->szFileOffset[1]) || lpXdbRec->szFileOffset[1] == FOM_LAST_SECTION) {
			if (isxdigit(lpXdbRec->szFileOffset[1])) {
				iSectionIdx = HexCharToInt(lpXdbRec->szFileOffset[1]);
				if (iSectionIdx >= lpPeHdr->peHdr32.wNumberOfSections)
					return FL_CLEAN;
			}
			else
				iSectionIdx = lpPeHdr->peHdr32.wNumberOfSections - 1;

			//if (lpXdbRec->szFileOffset[1] == '+' || lpXdbRec->szFileOffset[1] == '-')
			//    iDisp = strtol (&lpXdbRec->szFileOffset[2], NULL, 16);
			if (lpXdbRec->szFileOffset[2] == '+' || lpXdbRec->szFileOffset[2] == '-')
				iDisp = strtol(&lpXdbRec->szFileOffset[2], NULL, 16);

			if (PE_ParseSectionHeader(handle, iSectionIdx, &peSectionHdr) == -1)
				return FL_EPE;
		}
		else
			return FL_EFVD;

		_lseek(handle, peSectionHdr.dwPointerToRawData + iDisp, SEEK_SET);
	}
	break;

	case FVD_FO_EP:
	{
		int i;
		unsigned int dwEntryPointOffset = 0;
		if (lpPeHdr == NULL)
			return FL_EFVD;

		/* Find which section the entry point resides */
		for (i = 0; i < lpPeHdr->peHdr32.wNumberOfSections; ++i) {
			if (PE_ParseSectionHeader(handle, i, &peSectionHdr) == -1)
				return FL_EPE;
			if (lpPeHdr->peHdr32.dwAddressOfEntryPoint <
				(peSectionHdr.dwVirtualAddress + ALIGN(peSectionHdr.dwVirtualSize,
				lpPeHdr->peHdr32.dwSectionAlignment)))
			{
				/* Convert dwAddressOfEntryPoint's RVA to file offset */
				dwEntryPointOffset = peSectionHdr.dwPointerToRawData +
					(lpPeHdr->peHdr32.dwAddressOfEntryPoint - peSectionHdr.dwVirtualAddress);
				break;
			}
		}
		if (dwEntryPointOffset == 0)
			return FL_EPE;

		//if (lpXdbRec->szFileOffset[1] == '+')
		//    iDisp = strtol (&lpXdbRec->szFileOffset[2], NULL, 16);
		if (lpXdbRec->szFileOffset[1] == '+' || lpXdbRec->szFileOffset[1] == '-')
			iDisp = strtol(&lpXdbRec->szFileOffset[1], NULL, 16);
		else
			return FL_EFVD;

		_lseek(handle, dwEntryPointOffset + iDisp, SEEK_SET);
	}
	break;

	case FVD_FO_EOF:
	{
		if (lpXdbRec->szFileOffset[1] == '-')
			iDisp = strtol(&lpXdbRec->szFileOffset[2], NULL, 16);
		else
			return FL_EFVD;
		_lseek(handle, iDisp, SEEK_END);
	}
	break;

	default: /* szFileOffset is a hex number */
	{
		iDisp = strtoul(lpXdbRec->szFileOffset, NULL, 16);
		_lseek(handle, iDisp, SEEK_SET);
	}
	}

	buffer = (char*)malloc(lpXdbRec->dwSignLength);
	if (buffer == NULL)
		return FL_ENOMEM;
	_read(handle, buffer, lpXdbRec->dwSignLength);
	dwBuffCRC32 = Crc32_Asm(buffer, lpXdbRec->dwSignLength, 0);
	if (dwBuffCRC32 == lpXdbRec->dwSignCRC32) {
		strcpy_s(lpFvdBlock->szMalwareName, _countof(lpFvdBlock->szMalwareName), lpXdbRec->szMalwareName);
		lpFvdBlock->bMalwareType = lpXdbRec->bMalwareType;
		free(buffer);
		return FL_VIRUS;
	}
	free(buffer);
	return FL_CLEAN;
}
//
//static int GetArchiveType (const _TCHAR* szFilePathName)
//{
//    int handle, ret = ARC_NONE;
//    char buffer[8];
//    handle = _topen (szFilePathName, _O_RDONLY | _O_BINARY);
//    if (handle == -1)
//        return -1;
//
//    if (_read (handle, buffer, sizeof(buffer)) > 0) {
//        if (PTR(unsigned int, buffer[0]) == ZIP_SIGNATURE)
//            ret = ARC_ZIP;
//        else if (PTR(unsigned short, buffer[0]) == GZIP_SIGNATURE)
//            ret = ARC_GZIP;
//        else if (memcmp (buffer, SEVENZIP_SIGNATURE, sizeof(SEVENZIP_SIGNATURE)) == 0)
//            ret = ARC_7Z;
//    }
//
//    _close (handle);
//    return ret;
//}

/* EXPORTED FUNCTIONS */


int Sca_ScanFile(const _TCHAR* szFilePathName, /*LPSCA_OPT lpScaOpt,*/ LPFVD_BLOCK lpFvdBlock)
{
	int handle, ret = FL_CLEAN;
	int iFileType;

	PE_HEADER peHdr;
	unsigned int i;
	FVD_FIND_CWDBREC fvdFindCwdbRec;
	LPFVD_XDBREC aXdbRec;
	LPFVD_SDBREC aSdbRec;
	unsigned int dwXdbRecCount, dwSdbRecCount;

	if (_taccess(szFilePathName, 0) != 0) {
		return FL_ENOFILE;
	}

	handle = _tsopen(szFilePathName, _O_RDONLY | _O_BINARY, _SH_DENYWR);
	if (handle == -1) {
		return FL_EOPEN;
	}

	/* Check if file is a PE32 file */
	iFileType = PE_GetType(handle);
	if (iFileType < 1) {
		/* TODO: Check if file is a DOS exe file */
		/* TODO: Check if file is a script file */
		iFileType = FVD_TT_SCRIPT;
	}

	/* Scan file based on its file type */

	/* Scan with WDB Table */
	/* Fill information in fvdFindCwdbRec */
	fvdFindCwdbRec.bTblType = 1;
	fvdFindCwdbRec.dwFileSize = _filelength(handle);
	fvdFindCwdbRec.dwCRC32 = 0;
	/* Get crc32 value from the file */
	_lseek(handle, 0, SEEK_SET);
	Crc32File_Handle(handle, fvdFindCwdbRec.dwFileSize, &fvdFindCwdbRec.dwCRC32);
	/* Check if information in fvdFindCdbRec matches within database */
	if (Fvd_LookupCWDB(&fvdFindCwdbRec, lpFvdBlock))
		goto LReturn;

	/* Scan with CDB Table */
	fvdFindCwdbRec.bTblType = 0;
	/* Check if information in fvdFindCdbRec matches within database */
	if (Fvd_LookupCWDB(&fvdFindCwdbRec, lpFvdBlock)) {
		ret = FL_VIRUS;
		goto LReturn;
	}

	if (iFileType == FVD_TT_PE || iFileType == FVD_TT_PE64) {
		/* Parse PE Header */
		ret = PE_ParseHeader(handle, &peHdr);
		if (ret == -1) {
			ret = FL_EPE;
			goto LReturn;
		}
	}

	/* Scan with XDB Table */
	/* Always check "raw" file type first */
	aXdbRec = (LPFVD_XDBREC)Fvd_GetTblPtr(FVD_XDBTBL, FVD_TT_ALL, lpFvdBlock);
	dwXdbRecCount = Fvd_GetTblRecCount(FVD_XDBTBL, FVD_TT_ALL, lpFvdBlock);
	if (aXdbRec != NULL && dwXdbRecCount != 0) {
		for (i = 0; i < dwXdbRecCount; ++i) {
			ret = XDBScan(handle, &aXdbRec[i], &peHdr, lpFvdBlock);
			if (ret != FL_CLEAN)
				goto LReturn;
		}
	}

	/* Next check specified file type */
	aXdbRec = (LPFVD_XDBREC)Fvd_GetTblPtr(FVD_XDBTBL, iFileType, lpFvdBlock);
	dwXdbRecCount = Fvd_GetTblRecCount(FVD_XDBTBL, iFileType, lpFvdBlock);
	if (aXdbRec != NULL && dwXdbRecCount != 0) {
		for (i = 0; i < dwXdbRecCount; ++i) {
			ret = XDBScan(handle, &aXdbRec[i], &peHdr, lpFvdBlock);
			if (ret != FL_CLEAN)
				goto LReturn;
		}
	}

	/* Scan with SDB Table */
	/* Always check "All" file type first */
	aSdbRec = (LPFVD_SDBREC)Fvd_GetTblPtr(FVD_SDBTBL, FVD_TT_ALL, lpFvdBlock);
	dwSdbRecCount = Fvd_GetTblRecCount(FVD_SDBTBL, FVD_TT_ALL, lpFvdBlock);
	if (aSdbRec != NULL && dwSdbRecCount != 0) {
		for (i = 0; i < dwSdbRecCount; ++i) {
			ret = SDBScan(handle, (LPFVD_SDBREC)aSdbRec, iFileType, &peHdr, lpFvdBlock);
			if (ret != FL_CLEAN)
				goto LReturn;
			(char*)aSdbRec += aSdbRec->wRecSize;
		}
	}

	/* Next check specified file type */
	aSdbRec = (LPFVD_SDBREC)Fvd_GetTblPtr(FVD_SDBTBL, iFileType, lpFvdBlock);
	dwSdbRecCount = Fvd_GetTblRecCount(FVD_SDBTBL, iFileType, lpFvdBlock);
	if (aSdbRec != NULL && dwSdbRecCount != 0) {
		for (i = 0; i < dwSdbRecCount; ++i) {
			ret = SDBScan(handle, aSdbRec, iFileType, &peHdr, lpFvdBlock);
			if (ret != FL_CLEAN)
				goto LReturn;
			(char*)aSdbRec += aSdbRec->wRecSize;
		}
	}

	/* Experiment (still need improvement): If file is an executable file (not a dll), scan its resource */
	//if ((peHdr.peHdr32.wCharacteristics & IMAGE_FILE_DLL) == 0 && 
	//    (iFileType == FVD_TT_PE || iFileType == FVD_TT_PE64)) {
	//    /* Scan resource directory */
	//    ret = RsrcScan (handle, &peHdr, lpFvdBlock);
	//    if (ret != FL_CLEAN)
	//        goto LReturn;
	//}

LReturn:
	_close(handle);
	return ret;
}

//static int GenTempFileName (_TCHAR* szTempFilePath)
//{
//    _TCHAR* szEnv;
//    _TCHAR szTmpNam[13];
//
//    /* Get temporary folder path */
//    /* First try, uses environment variable */
//#ifdef _WIN32
//    szEnv = _tgetenv (_T("TEMP"));
//#else
//    szEnv = _tgetenv (_T("TEMPDIR"));
//#endif
//
//    if (szEnv != NULL) {
//        _tcscpy (szTempFilePath, szEnv);
//    }
//    else {
//        /* Second try, uses default directory */
//        _tcscpy (szTempFilePath, _T(DEFTEMPDIR));
//        if (_taccess(szTempFilePath, 6) != 0) {
//#ifdef _WIN32
//            /* maybe it's win2k */
//            _tcscpy (szTempFilePath, _T("C:\\WINNT\\TEMP"));
//            if (_taccess(szTempFilePath, 6) != 0)
//#endif
//                szTempFilePath[0] = '\0';
//        }
//    }
//
//    /* If all failed, uses current directory */
//    _ttmpnam (szTmpNam);
//    _tcscat (szTempFilePath, szTmpNam);
//    return 0;
//}

//int Sca_ScanFile (const _TCHAR* szFilePathName, /*LPSCA_OPT lpScaOpt,*/ LPFVD_BLOCK lpFvdBlock) 
//{
//    int ret = FL_CLEAN;
//
//    if (_taccess (szFilePathName, 0) != 0)
//        return FL_ENOFILE;
//
//    /* If lpScaOpt->fScanArchive == 1, scan archive file too if it is */
//    /*if (lpScaOpt->archive_opt.fScanArchive) {
//        ret = GetArchiveType (szFilePathName);
//        if (ret > 0) {
//            int handle;
//            handle = _topen (szFilePathName, _O_RDONLY | _O_BINARY);
//            if (_filelength (handle) > lpScaOpt->archive_opt.dwMaxScanArcSize)
//                return FL_CLEAN;
//            _close (handle);
//
//            --lpScaOpt->archive_opt.dwMaxSubArcExtract;
//            if (lpScaOpt->archive_opt.dwMaxSubArcExtract < 0)
//                return FL_CLEAN;
//
//            switch (ret)
//            {
//            case ARC_ZIP:
//                ret = ScanZipFile (szFilePathName, lpScaOpt, lpFvdBlock);
//                break;
//
//            default:
//                ret = ScanFile (szFilePathName, lpScaOpt, lpFvdBlock);
//            }
//        }
//        else
//            ret = ScanFile (szFilePathName, lpScaOpt, lpFvdBlock);
//    }
//    else*/
//        ret = ScanFile (szFilePathName, /*lpScaOpt,*/ lpFvdBlock);
//    return ret;
//}
