#include "main.h"
#include <iostream>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
using namespace std;

void WinAPIError();
bool inject(const char* procName, const char* dllPath);
DWORD procNameToPID(const char* procName);
bool dllInjection(DWORD pid, string dll);

int main()
{
    string dllPath;
    string processName;

    cout << "Enter the path to the DLL (e.g., C:\\Path\\To\\DLL.dll): ";
    getline(cin, dllPath);

    cout << "Enter the name of the process to inject into (e.g., explorer.exe): ";
    getline(cin, processName);

    if (dllPath.empty() || processName.empty())
    {
        cerr << "Error: DLL path or process name is empty." << endl;
        return 1;
    }

    cout << "\nDLL Path: " << dllPath << endl;
    cout << "Process Name: " << processName << endl;

    if (inject(processName.c_str(), dllPath.c_str()))
    {
        cout << "Injection successful!" << endl;
    }
    else
    {
        cerr << "Injection failed." << endl;
    }

    return 0;
}

bool inject(const char* procName, const char* dllPath)
{
    DWORD pid = procNameToPID(procName);

    cout << "Searching for process '" << procName << "'..." << endl;

    if (!pid)
    {
        cerr << "Error: Process not found." << endl;
        return false;
    }

    cout << "PID Found: " << pid << ". Attempting injection..." << endl;

    if (dllInjection(pid, dllPath))
    {
        cout << "Injection succeeded!" << endl;
        return true;
    }
    else
    {
        cerr << "Injection failed." << endl;
        return false;
    }
}

DWORD procNameToPID(const char* procName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        cerr << "Error: Unable to create snapshot." << endl;
        return 0;
    }

    PROCESSENTRY32 process;
    process.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &process))
    {
        do
        {
            if (strcmp(process.szExeFile, procName) == 0)
            {
                CloseHandle(snapshot);
                return process.th32ProcessID;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);
    return 0;
}

bool dllInjection(DWORD pid, string dll)
{
    HMODULE kernel32 = GetModuleHandle("Kernel32");
    FARPROC loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");

    HANDLE token;
    TOKEN_PRIVILEGES tokenPriv;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
    {
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tokenPriv.Privileges[0].Luid);
        tokenPriv.PrivilegeCount = 1;
        tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (!AdjustTokenPrivileges(token, FALSE, &tokenPriv, sizeof(tokenPriv), NULL, NULL))
        {
            WinAPIError();
            cerr << "Error: Could not adjust token privileges." << endl;
            return false;
        }
    }
    else
    {
        cerr << "Error: Cannot open process token." << endl;
        return false;
    }

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!processHandle)
    {
        cerr << "Error: Cannot open process." << endl;
        return false;
    }

    LPVOID remoteDllName = VirtualAllocEx(processHandle, NULL, dll.size() + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!remoteDllName)
    {
        CloseHandle(processHandle);
        WinAPIError();
        cerr << "Error: Could not allocate remote memory." << endl;
        return false;
    }

    SIZE_T writtenBytes;
    WriteProcessMemory(processHandle, remoteDllName, dll.c_str(), dll.size() + 1, &writtenBytes);

    HANDLE remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibrary, remoteDllName, 0, NULL);
    if (!remoteThread)
    {
        VirtualFreeEx(processHandle, remoteDllName, dll.size() + 1, MEM_RELEASE);
        CloseHandle(processHandle);
        WinAPIError();
        cerr << "Error: Could not create remote thread." << endl;
        return false;
    }

    bool finished = WaitForSingleObject(remoteThread, 10000) != WAIT_TIMEOUT;

    VirtualFreeEx(processHandle, remoteDllName, dll.size() + 1, MEM_RELEASE);
    CloseHandle(processHandle);

    if (!finished)
    {
        cerr << "Error: Timeout while waiting for the thread." << endl;
        return false;
    }

    return true;
}

void WinAPIError()
{
    LPSTR errorMessage = NULL;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, GetLastError(), 0, (LPSTR)&errorMessage, 0, 0);
    cerr << "WinAPI Error: " << errorMessage;
    LocalFree(errorMessage);
}