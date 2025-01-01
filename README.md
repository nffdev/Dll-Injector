# Dll-Injector
A simple tool for DLL injection into target processes on Windows.

## Requirements
- Visual Studio 2019 or higher
- Windows SDK (for access to Windows APIs)

## Installation
1. Clone the repository :
   ```bash
   git clone https://github.com/nffdev/Dll-Injector.git
   ```
2. Open the project in Visual Studio.
3. Compile the project using the `Release` or `Debug` configuration.

## Use
1. Compile and run the application.
2. When the program asks for the path of the DLL file, enter the full path of the DLL you wish to inject.
3. Enter the name of the target process (e.g. `explorer.exe` or `notepad.exe`).
4. The program will perform the injection and let you know if it was successful.

## TECHNICAL DETAILS

- **OpenProcessToken**: Used to open the current process token to adjust privileges.  
- **AdjustTokenPrivileges**: Ensures the necessary privileges (e.g., SE_DEBUG_NAME) are enabled to interact with other processes.  
- **CreateToolhelp32Snapshot**: Captures a snapshot of the system’s processes to find the target process by its name.  
- **OpenProcess**: Opens the target process with `PROCESS_ALL_ACCESS` to allow memory manipulation.  
- **VirtualAllocEx**: Allocates memory in the target process for storing the path of the DLL to inject.  
- **WriteProcessMemory**: Writes the path of the DLL into the allocated memory within the target process.  
- **GetProcAddress & LoadLibraryA**: Retrieves the address of `LoadLibraryA` to load the DLL into the target process.  
- **CreateRemoteThread**: Creates a new thread in the target process to execute `LoadLibraryA`, effectively loading the DLL.  
- **WaitForSingleObject**: Waits for the remote thread to finish execution, ensuring the DLL is loaded before freeing memory.

## Resources
- [Official Microsoft documentation on Windows APIs](https://docs.microsoft.com/en-us/windows/win32/)

## Demo

![Demo](https://raw.githubusercontent.com/nffdev/Dll-Injector/main/demo.gif)
