#ifndef _MAIN_H_
#define _MAIN_H_

#include <Windows.h>
#include <tlhelp32.h> 
#include <iostream>

using namespace std;

#define MSGRET(str, ret) { cout << "ERROR: " << str << endl; system("pause"); return ret; }

int main();
bool inject(const char* procName, const char* dllPath);
DWORD procNameToPID(const char* procName);
bool dllInjection(DWORD pid, string dll);
void WinAPIError();

#endif /* _MAIN_H_ */