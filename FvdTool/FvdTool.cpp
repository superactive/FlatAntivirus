/****************************************************************************\
*
*	FVD Tool for processing FVD file (FlatAV Database File)
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include "..\libflatav\flatav.h"
#include "path.h"

#define ACT_BUILDNEWDB  1
#define ACT_VALIDATEDB  2

void ShowHelp()
{
    _tprintf(_TEXT("FvdTool, created by Suyandi Wu (c) 2008\n"));
    _tprintf(_TEXT("Usage: FvdTool <Option> [List_of_Arguments]\n\n"));
    _tprintf(_TEXT("Option:\n"));
    _tprintf(_TEXT("  -b: Build new database.\n"));
    _tprintf(_TEXT("      Args: [outputdbfilename] [inputdir] [inputname]\n"));
    _tprintf(_TEXT("      Default values:\n"));
    _tprintf(_TEXT("        outdbname = main.fvd\n"));
    _tprintf(_TEXT("        inputdir = <current directory>\n"));
    _tprintf(_TEXT("        inputname = main\n"));
    _tprintf(_TEXT("  -c: Validate database.\n"));
    _tprintf(_TEXT("      Args: dbname [Minimum AV Mayor Ver] [Minimum AV Minor Ver]\n"));
    _tprintf(_TEXT("  -v: Verbose.\n"));
}

int CreateFvdErrorMessage(int nErrCode, _TCHAR** ppszRes)
{
    *ppszRes = (_TCHAR*) malloc(64 * sizeof(_TCHAR));
    if (*ppszRes == NULL) return -1;

    switch (nErrCode)
    {
    case 0: _tcscpy(*ppszRes, _TEXT("No error found.")); break;
    case FL_ENOFILE: _tcscpy(*ppszRes, _TEXT("No such file or directory.")); break;
    case FL_EIO: _tcscpy(*ppszRes, _TEXT("Input/output error.")); break;
    case FL_EBADF: _tcscpy(*ppszRes, _TEXT("Bad file descriptor.")); break;
    case FL_ENOMEM: _tcscpy(*ppszRes, _TEXT("Not enough memory.")); break;
    case FL_EACCESS: _tcscpy(*ppszRes, _TEXT("Permission denied.")); break;
    case FL_EMFILE: _tcscpy(*ppszRes, _TEXT("Too many open files.")); break;
    case FL_EOPEN: _tcscpy(*ppszRes, _TEXT("File open error.")); break;
    case FL_EWRITE: _tcscpy(*ppszRes, _TEXT("File write error.")); break;
    case FL_ENULLARG: _tcscpy(*ppszRes, _TEXT("NULL argument passed.")); break;
    case FL_ETEMPF: _tcscpy(*ppszRes, _TEXT("Make temp file failed.")); break;
    case FL_EFVD: _tcscpy(*ppszRes, _TEXT("Invalid fvd format.")); break;
    case FL_EPE: _tcscpy(*ppszRes, _TEXT("Invalid PE file.")); break;
    case FL_EARC: _tcscpy(*ppszRes, _TEXT("Invalid archive file.")); break;
    case FL_ENOTSUPT: _tcscpy(*ppszRes, _TEXT("Operation not supported.")); break;

    case FVD_EWRITE_FVD: _tcscpy(*ppszRes, _TEXT("Cannot write to destination path.")); break;
    case FVD_EOPEN_HDB: _tcscpy(*ppszRes, _TEXT("Cannot open HDB file.")); break;
    case FVD_EINV_HDB: _tcscpy(*ppszRes, _TEXT("Invalid HDB file.")); break;
    case FVD_EACCESS_HDB: _tcscpy(*ppszRes, _TEXT("Cannot access HDB file.")); break;
    case FVD_EOPEN_CDB: _tcscpy(*ppszRes, _TEXT("Cannot open CDB file.")); break;
    case FVD_EINV_CDB: _tcscpy(*ppszRes, _TEXT("Invalid CDB file.")); break;
    case FVD_EACCESS_CDB: _tcscpy(*ppszRes, _TEXT("Cannot access CDB file.")); break;
    case FVD_EOPEN_XDB: _tcscpy(*ppszRes, _TEXT("Cannot open XDB file.")); break;
    case FVD_EINV_XDB: _tcscpy(*ppszRes, _TEXT("Invalid XDB file.")); break;
    case FVD_EACCESS_XDB: _tcscpy(*ppszRes, _TEXT("Cannot access XDB file.")); break;
    case FVD_EOPEN_SDB: _tcscpy(*ppszRes, _TEXT("Cannot open SDB file.")); break;
    case FVD_EINV_SDB: _tcscpy(*ppszRes, _TEXT("Invalid SDB file.")); break;
    case FVD_EACCESS_SDB: _tcscpy(*ppszRes, _TEXT("Cannot access SDB file.")); break;
    case FVD_EOPEN_WDB: _tcscpy(*ppszRes, _TEXT("Cannot open WDB file.")); break;
    case FVD_EINV_WDB: _tcscpy(*ppszRes, _TEXT("Invalid WDB file.")); break;
    case FVD_EACCESS_WDB: _tcscpy(*ppszRes, _TEXT("Cannot access WDB file.")); break;
    case FVD_EWRITE_CHECKSUM: _tcscpy(*ppszRes, _TEXT("Cannot create checksum.")); break;
    case FVD_EINV_MARK: _tcscpy(*ppszRes, _TEXT("Invalid FVD mark.")); break;
    case FVD_EMIN_AV_VER: _tcscpy(*ppszRes, _TEXT("Minimal AV version not satisfied.")); break;
    case FVD_EINV_CHECKSUM: _tcscpy(*ppszRes, _TEXT("Invalid checksum.")); break;
    case FVD_EPARAM: _tcscpy(*ppszRes, _TEXT("Invalid parameter.")); break;
    default: _tcscpy(*ppszRes, _TEXT("Unknown error."));
    }
    return 0;
}

int BuildNewDb(_TCHAR* outdbname, _TCHAR* inputdir, _TCHAR* inputname)
{
    int nRet = 0;
    _TCHAR cdbFPN[_MAX_PATH], sdbFPN[_MAX_PATH], xdbFPN[_MAX_PATH], wdbFPN[_MAX_PATH],
        hdbFPN[_MAX_PATH];
    _TCHAR* buff = NULL;
    _TCHAR* pOutdbname;
    _TCHAR* pInputname = _TEXT("main");
    _TCHAR* pInputdir = _TEXT("");

    //_tprintf(_T("%s\n"), outdbname);
    //_tprintf(_T("%s\n"), inputdir);
    //_tprintf(_T("%s\n"), inputname);

    if (inputdir)
        pInputdir = inputdir;

    if (inputname)
        pInputname = inputname;

    if (outdbname)
        pOutdbname = outdbname;
    else {
        buff = (_TCHAR*) malloc((_tcslen(pInputdir) + 6 + _tcslen(pInputname)) * sizeof(_TCHAR));
        if (buff == NULL) return FL_ENOMEM;

        if (inputdir) {
            _tcscpy(buff, pInputdir);
            PathAddBkSlash(buff);
            if (inputname) {
                _tcscat(buff, pInputname);
                _tcscat(buff, _TEXT(".fvd"));
            }
            else
                _tcscat(buff, _TEXT("main.fvd"));
        }
        else {
            if (inputname) {
                _tcscpy(buff, pInputname);
                _tcscat(buff, _TEXT(".fvd"));
            }
            else
                _tcscpy(buff, _TEXT("main.fvd"));
        }
        pOutdbname = buff;
    }

    _tcscpy(hdbFPN, pInputdir);
    if (hdbFPN[0]) PathAddBkSlash(hdbFPN);
    _tcscat(hdbFPN, pInputname);
    _tcscat(hdbFPN, _TEXT(".hdb"));

    _tcscpy(cdbFPN, pInputdir);
    if (cdbFPN[0]) PathAddBkSlash(cdbFPN);
    _tcscat(cdbFPN, pInputname);
    _tcscat(cdbFPN, _TEXT(".cdb"));

    _tcscpy(sdbFPN, pInputdir);
    if (sdbFPN[0]) PathAddBkSlash(sdbFPN);
    _tcscat(sdbFPN, pInputname);
    _tcscat(sdbFPN, _TEXT(".sdb"));

    _tcscpy(xdbFPN, pInputdir);
    if (xdbFPN[0]) PathAddBkSlash(xdbFPN);
    _tcscat(xdbFPN, pInputname);
    _tcscat(xdbFPN, _TEXT(".xdb"));

    _tcscpy(wdbFPN, pInputdir);
    if (wdbFPN[0]) PathAddBkSlash(wdbFPN);
    _tcscat(wdbFPN, pInputname);
    _tcscat(wdbFPN, _TEXT(".wdb"));

    if (_taccess(hdbFPN, 0) == -1)
        nRet = FVD_EACCESS_HDB;
    else {
        nRet = Fvd_BuildNewDb(
            hdbFPN, 
            _taccess(cdbFPN, 0) == 0 ? cdbFPN : NULL, 
            _taccess(xdbFPN, 0) == 0 ? xdbFPN : NULL, 
            _taccess(sdbFPN, 0) == 0 ? sdbFPN : NULL, 
            _taccess(wdbFPN, 0) == 0 ? wdbFPN : NULL,
            pOutdbname);
    }
    if (buff) free(buff);
    return nRet;
}

int ValidateDb(_TCHAR* dbname, unsigned short wCurMinAvMayorVersion, 
               unsigned char bCurMinAvMinorVersion)
{
    int nRet = 0;

    FVD_BLOCK fvdBlock;
    nRet = Fvd_LoadDb(dbname, &fvdBlock);
    if (nRet != 0)
        return nRet;

    nRet = Fvd_ValidateDb(&fvdBlock, wCurMinAvMayorVersion, bCurMinAvMinorVersion);
    if (nRet != 0)
        return nRet;

    Fvd_UnloadDb(&fvdBlock);
    return nRet;
}

int _tmain(int argc, _TCHAR* argv[])
{
    int nRet = 0;
    int bVerbose = 0, bPaused = 0;
    if (argc < 2)
        ShowHelp();
    else {
        int i, nAction = 0;
        unsigned int j;
        for (i = 1; i < argc; ++i) {
            if (argv[i][0] == '-' || argv[i][0] == '/') {
                size_t len = _tcslen(argv[i]);
                for (j = 1; j < len; ++j) {
                    switch (tolower(argv[i][j]))
                    {
                        case 'b': nAction = ACT_BUILDNEWDB; break;
                        case 'c': nAction = ACT_VALIDATEDB; break;
                        case 'v': bVerbose = 1; break;
                        case 'p': bPaused = 1; break;
                        default: ShowHelp(); return -1;
                    }// END switch
                }//END for (j = 1; ...
            }//END if (argv[i][0] == '-' || argv[i][0] == '/')
            else
                break;
        }//END for (i = 1; ...

        if (nAction == 0) {
            ShowHelp();
        }
        else {
            int nArgCount = argc - i; 
            switch (nAction)
            {
                case ACT_BUILDNEWDB:
                    {
                        switch (nArgCount)
                        {
                            case 0: nRet = BuildNewDb(NULL, NULL, NULL); break;
                            case 1: nRet = BuildNewDb(argv[i], NULL, NULL); break;
                            case 2: nRet = BuildNewDb(argv[i], argv[i+1], NULL); break;
                            case 3: default:
                                nRet = BuildNewDb(argv[i], argv[i+1], argv[i+2]); break;
                        }
                    }
                    break;

                case ACT_VALIDATEDB:
                    {
                        switch (nArgCount)
                        {
                            case 0: ShowHelp(); return -1;
                            case 1: nRet = ValidateDb(argv[i], 0, 0); break;
                            case 2: nRet = ValidateDb(argv[i], _ttoi(argv[i+1]), 0); break;
                            case 3: default:
                                nRet = ValidateDb(argv[i], _ttoi(argv[i+1]), _ttoi(argv[i+2])); break;
                        }
                    }
                    break;

            }// END switch (nAction)

            if (bVerbose) {
                if (nRet == 0) {
                    switch (nAction)
                    {
                        case ACT_BUILDNEWDB: _tprintf(_TEXT("Database successfully created.\n")); break;
                        case ACT_VALIDATEDB: _tprintf(_TEXT("Database is valid.\n")); break;
                    }
                }
                else {
                    _TCHAR* p;
                    CreateFvdErrorMessage(nRet, &p);
                    _tprintf(_TEXT("%s\n"), p);
                    free (p);
                }
            }// END if (bVerbose)
        }// END else FROM if (nAction == 0)

    }// END else FROM if (argc < 2)
    if (bPaused) system("pause");
    return nRet;
}
