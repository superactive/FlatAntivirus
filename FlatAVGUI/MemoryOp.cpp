/****************************************************************************\
*
*	Memory process functions
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include <windows.h>
#include <tlhelp32.h> 

BOOL SuspendResumeProcess (DWORD dwProcessId, BOOL bResume)
{
    BOOL bRet = FALSE;
    THREADENTRY32 threadEntry32;

    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot (TH32CS_SNAPTHREAD, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    threadEntry32.dwSize = sizeof(THREADENTRY32);
    if (::Thread32First (hProcessSnap, &threadEntry32)) {
        do {
            if (threadEntry32.th32OwnerProcessID == dwProcessId) {
                HANDLE hThread = ::OpenThread (THREAD_SUSPEND_RESUME, FALSE, 
                    threadEntry32.th32ThreadID);
                if (hThread) {
                    if (bResume)
                        bRet = (::ResumeThread (hThread) != 0xffffffff);
                    else {  // suspend more than 1 time
                        ::SuspendThread (hThread);
                        bRet = (::SuspendThread (hThread) != 0xffffffff);
                    }
                    ::CloseHandle (hThread);
                }
                break;
            }
        } while (::Thread32Next (hProcessSnap, &threadEntry32));
    }
    ::CloseHandle (hProcessSnap);
    return bRet;
}

BOOL KillProcess (DWORD dwProcessId)
{
    BOOL bRet = FALSE;
    HANDLE hProcess = ::OpenProcess (PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, 
        FALSE, dwProcessId);
    if (hProcess) {
        bRet = ::TerminateProcess (hProcess, 0);
        if (bRet) {
            DWORD dwExitCode;
            while (::GetExitCodeProcess (hProcess, &dwExitCode) && 
                dwExitCode == STILL_ACTIVE);
            //::WaitForSingleObject (hProcess, INFINITE);
        }
        ::CloseHandle (hProcess);
    }
    return bRet;
}

BOOL StopAndDeleteService (LPCTSTR szServiceName, BOOL bIsActive)
{
    BOOL bRet = FALSE;
    if (szServiceName[0] != '\0') {
        SC_HANDLE hSCManager = ::OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCManager) {
            SC_HANDLE hService = ::OpenService (hSCManager, szServiceName, SERVICE_STOP | DELETE);
            if (hService) {
                // Active service's process id won't be zero
                if (bIsActive) {
                    SERVICE_STATUS st;
                    ::ControlService (hService, SERVICE_CONTROL_STOP, &st);
                }
                bRet = ::DeleteService (hService);
                ::CloseServiceHandle (hService);
            }
            ::CloseServiceHandle (hSCManager);
        }
    }
    return bRet;
}
