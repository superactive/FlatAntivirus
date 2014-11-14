/****************************************************************************\
*
*	For enumerating OS objects (memory process, start up items, registry keys, etc)
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include "StdAfx.h"
#include ".\EnumMWObjectArray.h"

CEnumMWObjectArray::CEnumMWObjectArray(/*BOOL bEnumObj /* = FALSE */)
{
	//if (bEnumObj)
	//    EnumObjects ();
}

CEnumMWObjectArray::~CEnumMWObjectArray()
{
	RemoveAllObjects();
}

void CEnumMWObjectArray::RemoveAllObjects()
{
	for (int i = 0; i < m_aptrMWObj.GetSize(); ++i)
		delete m_aptrMWObj.GetAt(i);
	m_aptrMWObj.RemoveAll();
}


BOOL CEnumMWObjectArray::IsObjAlreadyExists(LPCTSTR szFPN)
{
	int i;
	for (i = 0; i < m_aptrMWObj.GetSize(); ++i) {
		LPMWOBJ pMWObj = m_aptrMWObj.GetAt(i);
		if (::lstrcmpi(pMWObj->szFilePathName, szFPN) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL CEnumMWObjectArray::AddMWObject(LPTSTR szFPN, DWORD dwType, DWORD dwPID,
	LPTSTR szServiceName)
{
	ASSERT(szFPN);
	LPMWOBJ pMWObj = new MWOBJ;
	if (pMWObj == NULL) return FALSE;
	::ZeroMemory(pMWObj, sizeof(MWOBJ));
	pMWObj->dwType = dwType;
	::lstrcpy(pMWObj->szFilePathName, szFPN);
	switch (dwType)
	{
	case MWT_PROCESS:
		pMWObj->dwProcessId = dwPID;
		break;

	case MWT_ARCHIVE:
	case MWT_SERVICE:
		if (szServiceName) ::lstrcpy(pMWObj->szServiceName, szServiceName);
		break;
	}
	m_aptrMWObj.Add(pMWObj);
	return TRUE;
}

LPTSTR CEnumMWObjectArray::GetAbsPathFromFileSpec(LPTSTR szFileSpec, LPTSTR szOutput)
{
	LPTSTR szEnvPath, szEnvPathExt;
	DWORD dwSize = ::GetEnvironmentVariable(_T("PATH"), NULL, 0);
	szEnvPath = new TCHAR[dwSize + 1];
	if (szEnvPath) {
		if (::GetEnvironmentVariable(_T("PATH"), szEnvPath, dwSize)) {
			TCHAR szFPN[MAX_PATH];
			TCHAR* tokContext = NULL;
			PTSTR p = _tcstok_s(szEnvPath, _T(";"), &tokContext);
			while (p) {
				::lstrcpy(szFPN, p);
				::PathAddBackslash(szFPN);
				::lstrcat(szFPN, szFileSpec);

				if (!*::PathFindExtension(szFPN)) {
					DWORD dwFPNLen = ::lstrlen(szFPN);
					dwSize = ::GetEnvironmentVariable(_T("PATHEXT"), NULL, 0);
					szEnvPathExt = new TCHAR[dwSize + 1];
					if (szEnvPathExt) {
						if (::GetEnvironmentVariable(_T("PATHEXT"), szEnvPathExt, dwSize)) {
							int i;
							PTSTR p2 = szEnvPathExt;
							while (*p2) {
								if (*p2 != ';') {
									i = 0;
									while (*p2 && *p2 != ';') {
										szFPN[dwFPNLen + i] = *p2;
										++i;
										if ((dwFPNLen + i) == MAX_PATH) break;
										++p2;
									}
									szFPN[dwFPNLen + i] = '\0';

									if (::PathFileExists(szFPN)) {
										::lstrcpy(szOutput, szFPN);
										delete[] szEnvPathExt;
										delete[] szEnvPath;
										return szOutput;
									}
								}
								else ++p2;
							}
						}
						delete[] szEnvPathExt;
					}
				}

				if (::PathFileExists(szFPN)) {
					::lstrcpy(szOutput, szFPN);
					delete[] szEnvPath;
					return szOutput;
				}

				p = _tcstok_s(NULL, _T(";"), &tokContext);
			}
		}
		delete[] szEnvPath;
	}
	return NULL;
}

int CEnumMWObjectArray::EnumArgs(LPTSTR szPath)
{
	TCHAR szFPN[MAX_PATH];
	int i, argc = 0;
	LPTSTR pstr = ::PathGetArgs(szPath);
	while (*pstr) {
		if (*pstr != ' ') {
			if (*pstr == '"') {
				++pstr;
				i = 0;
				while (*pstr) {
					if (*pstr != '"') {
						szFPN[i++] = *pstr;
						++pstr;
						if (i == MAX_PATH) break;
					}
					else {
						++pstr;
						break;
					}
				}
				if (i != 0) {
					TCHAR szFPN2[MAX_PATH]; LPTSTR pstr = szFPN;
					szFPN[i] = '\0';
					if (::PathIsFileSpec(szFPN)) {
						GetAbsPathFromFileSpec(szFPN, szFPN2);
						pstr = szFPN2;
					}
					if (::PathFileExists(pstr))
						AddMWObjectEx(pstr);
				}
			}
			else {
				i = 0;
				while (*pstr && *pstr != ' ') {
					szFPN[i++] = *pstr;
					++pstr;
					if (i == MAX_PATH) break;
				}
				szFPN[i] = '\0';
				TCHAR szFPN2[MAX_PATH]; LPTSTR pstr = szFPN;
				if (::PathIsFileSpec(szFPN)) {
					GetAbsPathFromFileSpec(szFPN, szFPN2);
					pstr = szFPN2;
				}
				if (::PathFileExists(pstr))
					AddMWObjectEx(pstr);
			}
			++argc;
		}
		else ++pstr;
	}
	return argc;
}

void CEnumMWObjectArray::AddMWObjectEx(LPTSTR szPath)
{
	int nArgc = EnumArgs(szPath);
	if (nArgc > 0)
		::PathRemoveArgs(szPath);
	::PathUnquoteSpaces(szPath);

	TCHAR szFPN2[MAX_PATH]; LPTSTR pstr = szPath;
	if (::PathIsFileSpec(szPath)) {
		GetAbsPathFromFileSpec(szPath, szFPN2);
		pstr = szFPN2;
	}

	if (!IsObjAlreadyExists(pstr)) {
		if (::PathFileExists(pstr))
			AddMWObject(pstr);
	}
}

BOOL CEnumMWObjectArray::EnumObjects(DWORD dwEnumType,
	BOOL bRemoveAllExistingObj /* = FALSE */)
{
	BOOL bRes = TRUE;;
	if (bRemoveAllExistingObj)
		RemoveAllObjects();

	switch (dwEnumType)
	{
	default:
	case ET_ALL:
		EnumMemoryObjects();
		bRes = EnumStartupObjects();
		break;

	case ET_MEMORY: bRes = EnumMemoryObjects(); break;

	case ET_STARTUP: bRes = EnumStartupObjects(); break;
	}

	return bRes;
}

BOOL CEnumMWObjectArray::EnumMemoryObjects()
{
	int i, j;
	DWORD aPID[1024], dwBytesNeeded, dwProcessCount;
	TCHAR szModulePathName[MAX_PATH];
	LPMWOBJ pMWObj;

	//////////////////////////////
	//
	//  Processes
	//
	//////////////////////////////

	if (!::EnumProcesses(aPID, sizeof(aPID), &dwBytesNeeded))
		return FALSE;

	dwProcessCount = dwBytesNeeded / sizeof(DWORD);

	for (i = 0; (unsigned)i < dwProcessCount; ++i) {
		HMODULE arrhModule[1];

		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
			aPID[i]);
		if (hProcess == NULL)
			continue;

		if (::EnumProcessModules(hProcess, arrhModule, sizeof(arrhModule), &dwBytesNeeded)) {
			if (::GetModuleFileNameEx(hProcess, arrhModule[0], szModulePathName, MAX_PATH)) {
				AddMWObject(szModulePathName, MWT_PROCESS, aPID[i]);
			}   // if (::GetModuleFileNameEx (hProcess, arrhModule[j], szModulePathName, MAX_PATH))

		}   // if (::EnumProcessModules (hProcess, arrhModule, sizeof(arrhModule), &dwBytesNeeded))
		::CloseHandle(hProcess);
	}   // for (i = 0; (unsigned)i < dwProcessCount; ++i)

	//////////////////////////////
	//
	//  Services
	//
	//////////////////////////////

	// Get all active and inactive service path
	SC_HANDLE hSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL)
		goto L001;

	DWORD dwServicesNum, dwResumeHandle = 0;
	BOOL ret = ::EnumServicesStatus(hSCManager, SERVICE_WIN32, SERVICE_ACTIVE | SERVICE_INACTIVE,
		NULL, 0, &dwBytesNeeded, &dwServicesNum, &dwResumeHandle);

	if (!ret && GetLastError() != ERROR_MORE_DATA) {
		::CloseServiceHandle(hSCManager);
		goto L001;
	}

	LPENUM_SERVICE_STATUS pest = new ENUM_SERVICE_STATUS[dwBytesNeeded];

	dwResumeHandle = 0;
	ret = ::EnumServicesStatus(hSCManager, SERVICE_WIN32, SERVICE_ACTIVE | SERVICE_INACTIVE,
		pest, dwBytesNeeded, &dwBytesNeeded, &dwServicesNum, &dwResumeHandle);

	if (!ret) {
		delete pest;
		::CloseServiceHandle(hSCManager);
		goto L001;
	}

	LPQUERY_SERVICE_CONFIG pqsc = new QUERY_SERVICE_CONFIG[4096];
	// Iterate each service name and get its binary path
	for (i = 0; (unsigned)i < dwServicesNum; ++i) {
		SC_HANDLE hService = ::OpenService(hSCManager, pest[i].lpServiceName, SERVICE_ALL_ACCESS);
		if (hService) {
			ret = ::QueryServiceConfig(hService, pqsc, 4096, &dwBytesNeeded);
			if (ret) {
				// Remove any args and unquote it
				if (*::PathGetArgs(pqsc->lpBinaryPathName))
					::PathRemoveArgs(pqsc->lpBinaryPathName);
				::PathUnquoteSpaces(pqsc->lpBinaryPathName);

				// Make sure the filepath is not in array yet
				BOOL bAlreadyExists = FALSE;
				for (j = 0; j < m_aptrMWObj.GetSize(); ++j) {
					pMWObj = m_aptrMWObj.GetAt(j);
					if (::lstrcmpi(pMWObj->szFilePathName, pqsc->lpBinaryPathName) == 0) {
						bAlreadyExists = TRUE;
						break;
					}
				}
				if (!bAlreadyExists) {
					AddMWObject(pqsc->lpBinaryPathName, MWT_SERVICE, 0, pest[i].lpServiceName);
				}
				else {
					// If debug priv is enabled, service process can be killed directly, 
					//  so it'll be considered as process object, else as service object
					if (!IsDebugPrivEnabled())
						pMWObj->dwType = MWT_SERVICE;
					::lstrcpy(pMWObj->szServiceName, pest[i].lpServiceName);
				}
			}
			::CloseServiceHandle(hService);
		}
	}

	delete pqsc;
	delete pest;
	::CloseServiceHandle(hSCManager);

	//////////////////////////////
	//
	//  Modules
	//
	//////////////////////////////

L001:
	for (i = 0; (unsigned)i < dwProcessCount; ++i) {
		HMODULE arrhModule[1024];

		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
			aPID[i]);
		if (hProcess == NULL)
			continue;

		if (::EnumProcessModules(hProcess, arrhModule, sizeof(arrhModule), &dwBytesNeeded)) {
			for (j = 1; (unsigned)j < (dwBytesNeeded / sizeof(HMODULE)); ++j) {

				if (::GetModuleFileNameEx(hProcess, arrhModule[j], szModulePathName, MAX_PATH)) {
					// Make sure the filepath is not in array yet
					if (!IsObjAlreadyExists(szModulePathName)) {
						AddMWObject(szModulePathName, MWT_PROCESS, aPID[i]);
					}
				}   // if (::GetModuleFileNameEx (hProcess, arrhModule[j], szModulePathName, MAX_PATH))
			}   // for (j = 0; (unsigned)j < (dwBytesNeeded / sizeof(HMODULE)); ++j)
		}   // if (::EnumProcessModules (hProcess, arrhModule, sizeof(arrhModule), &dwBytesNeeded))
		::CloseHandle(hProcess);
	}   // for (i = 0; (unsigned)i < dwProcessCount; ++i)

	return TRUE;
}

BOOL CEnumMWObjectArray::EnumStartupObjects()
{
	EnumRegKeyWinlogon();
	EnumRegKeySafeBoot(_T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot"));
	EnumRegKeySafeBoot(_T("SYSTEM\\ControlSet001\\Control\\SafeBoot"));
	EnumRegKeySafeBoot(_T("SYSTEM\\ControlSet002\\Control\\SafeBoot"));
	EnumRegKeySafeBoot(_T("SYSTEM\\ControlSet003\\Control\\SafeBoot"));
	EnumRegKeyRun(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"));
	EnumRegKeyRun(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Runonce"));
	EnumRegKeyRun(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
	EnumRegKeyRun(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Runonce"));
	// TODO: scan link in startup folder
	return TRUE;
}

void CEnumMWObjectArray::EnumRegKeyRun(HKEY hRootKey, LPCTSTR szSubKey)
{
	HKEY hKey;
	int i = 0, nRet;
	TCHAR szValName[MAX_PATH]; LPTSTR pszData;
	DWORD cbValName = MAX_PATH, cbData;

	if (::RegOpenKeyEx(hRootKey, szSubKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD cbMaxValueLen = 0;
		if (::RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			&cbMaxValueLen, NULL, NULL) == 0) {

			cbData = cbMaxValueLen;
			pszData = new TCHAR[cbMaxValueLen + 1];
			if (pszData == NULL) {
				::RegCloseKey(hKey);
				return;
			}

			while ((nRet = ::RegEnumValue(hKey, i++, szValName, &cbValName, NULL, NULL,
				(LPBYTE)pszData, &cbData)) == ERROR_SUCCESS || nRet != ERROR_NO_MORE_ITEMS) {

				AddMWObjectEx(pszData);

				cbValName = MAX_PATH;
				cbData = cbMaxValueLen;
			}

			delete[] pszData;
		}
		::RegCloseKey(hKey);
	}
}

void CEnumMWObjectArray::EnumRegKeyWinlogon()
{
	HKEY hKey;
	LPTSTR pszData;
	DWORD cbData;

	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, KEY_READ, &hKey)
		== ERROR_SUCCESS) {

		DWORD cbMaxValueLen = 0;

		if (::RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			&cbMaxValueLen, NULL, NULL) == 0) {
			cbData = cbMaxValueLen;
			pszData = new TCHAR[cbMaxValueLen + 1];
			if (pszData == NULL) {
				::RegCloseKey(hKey);
				return;
			}

			TCHAR* tokContext = NULL;
			if (::RegQueryValueEx(hKey, _T("Userinit"), NULL, NULL, (LPBYTE)pszData, &cbData) == 0) {
				LPTSTR pstr = _tcstok_s(pszData, _T(","), &tokContext);
				while (pstr) {
					AddMWObjectEx(pstr);
					pstr = _tcstok_s(NULL, _T(","), &tokContext);
				}
			}

			cbData = cbMaxValueLen;
			if (::RegQueryValueEx(hKey, _T("Shell"), NULL, NULL, (LPBYTE)pszData, &cbData) == 0)
				AddMWObjectEx(pszData);

			cbData = cbMaxValueLen;
			if (::RegQueryValueEx(hKey, _T("System"), NULL, NULL, (LPBYTE)pszData, &cbData) == 0)
				AddMWObjectEx(pszData);

			delete[] pszData;
		}
		::RegCloseKey(hKey);
	}
}

void CEnumMWObjectArray::EnumRegKeySafeBoot(LPCTSTR szSubKey)
{
	HKEY hKey;
	LPTSTR pszData;
	DWORD cbData;

	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_READ, &hKey) == 0) {
		DWORD cbMaxValueLen = 0;
		if (::RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			&cbMaxValueLen, NULL, NULL) == 0) {
			cbData = cbMaxValueLen;
			pszData = new TCHAR[cbMaxValueLen + 1];
			if (pszData == NULL) {
				::RegCloseKey(hKey);
				return;
			}

			if (::RegQueryValueEx(hKey, _T("AlternateShell"), NULL, NULL, (LPBYTE)pszData, &cbData) == 0)
				AddMWObjectEx(pszData);

			delete[] pszData;
		}
		::RegCloseKey(hKey);
	}
}

BOOL CEnumMWObjectArray::IsDebugPrivEnabled()
{
	BOOL bResult;
	LUID luid;
	PRIVILEGE_SET ps;
	HANDLE hToken, hCurProcess;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) return FALSE;
	hCurProcess = GetCurrentProcess();
	if (OpenProcessToken(hCurProcess, TOKEN_QUERY, &hToken)) {
		ps.PrivilegeCount = 1;
		ps.Control = 0;
		ps.Privilege->Luid = luid;
		ps.Privilege->Attributes = SE_PRIVILEGE_ENABLED;
		if (!PrivilegeCheck(hToken, &ps, &bResult))
			bResult = FALSE;
		CloseHandle(hToken);
	}
	return bResult;
}