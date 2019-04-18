#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

//This function can be improved, will work on it whenever I can.
bool injectDll(uintptr_t processID, const char* dllPath) {

	//This will open a handle to the target process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

	//if the handle is valid then execute condition
	if (hProcess) {
		// This will allocate memory for the dllpath in the target process length of the path string + null terminator
		LPVOID loadPath = VirtualAllocEx(hProcess, 0, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

		// We Write the path to the address of the memory we just allocated in the target process
		WriteProcessMemory(hProcess, loadPath, (LPVOID)dllPath, strlen(dllPath) + 1, 0);

		// This will create a Remote Thread in the target process which calls LoadLibraryA as our dllpath as an argument -> program loads our dll
		HANDLE remoteThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"), loadPath, 0, 0);

		//We wait for the execution of our loader thread to finish
		WaitForSingleObject(remoteThread, INFINITE);

		// Free the memory allocated for our dll path
		VirtualFreeEx(hProcess, loadPath, strlen(dllPath) + 1, MEM_RELEASE);

		//Clean up and return true
		CloseHandle(remoteThread);
		CloseHandle(hProcess);
		return true;
	}
	//Return false if not successful
	return false;
}

uintptr_t getProcessID(const char* targetProcess, uintptr_t desiredAccess) {
	HANDLE hProcess = NULL;
	//Takes a snapshot of the specified processes, as well as the heaps, modules, and threads used by these processes. 
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	uintptr_t processID = NULL;
	//Check if snapshot created is valid
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to take a snapshot" << std::endl;
		return false;
	}


	PROCESSENTRY32 pEntry;
	//The size of the structure, in bytes. Before calling the Process32First function, set this
	//member to sizeof(PROCESSENTRY32). If you do not initialize dwSize, Process32First fails.
	pEntry.dwSize = sizeof(PROCESSENTRY32);

	//Loop through the processes
	do {
		//Compare the targetProcess with the process in pEntry.szExeFile (current process)
		//if the name of the process we are at right now matches the target process then we found it
		if (!strcmp(pEntry.szExeFile, targetProcess)) {
			//Process Found
			std::cout << "Found Process " << pEntry.szExeFile << " with process ID " << pEntry.th32ProcessID << std::endl;

			//Open the process with desired access and the process ID of the target process
			hProcess = OpenProcess(desiredAccess, FALSE, pEntry.th32ProcessID);
			processID = pEntry.th32ProcessID;
			CloseHandle(hSnapShot);

			//Check if handle value valid
			if (hProcess == INVALID_HANDLE_VALUE) {
				std::cout << "Failed getting a handle to the process!" << std::endl;
				return false;
			}
		}

		//Retrieves information about the first process encountered in a system snapshot.
		//Returns TRUE if the first entry of the process list has been copied to the buffer or FALSE otherwise.
	} while (Process32Next(hSnapShot, &pEntry));

	return processID;
}

int main() {

	injectDll(getProcessID("notepad++.exe", PROCESS_ALL_ACCESS), "C:\\Users\\memN0ps\\source\\repos\\dllmain\\Debug\\dllmain.dll");
	return 0;
}
