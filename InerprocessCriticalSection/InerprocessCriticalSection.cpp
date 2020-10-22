#include <iostream>
#include <windows.h>
#include <conio.h>
#include <io.h>
#include <tchar.h>

#define CRITICAL_SECTION_NAME TEXT("OLD_SECTION")
#define SHARED_MEMORY_NAME TEXT("SOMETHING")
#define N 3

LPCTSTR initSomeShit(HANDLE* mapping, size_t size, LPCWSTR name);
void freeResources(HANDLE* mapping, LPCTSTR* buff, CRITICAL_SECTION* section);

int main()
{
    size_t sizeOfSection = sizeof(CRITICAL_SECTION);
    size_t sizeOfSharedMemory = sizeof(int) * N;
    int arr[N] = { 1,2,3 };
    LPCTSTR bufForCrSection,bufForSharedMemory;
    HANDLE mappingForCrSection, mappingForMemory;
    CRITICAL_SECTION crSection;
    STARTUPINFO siProcFirst = { sizeof(STARTUPINFO) }, siProcSecond = { sizeof(STARTUPINFO) }, siProcThird = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION piProcFirst, piProcSecond, piProcThird;
    std::wstring CommandLineFirst(L"D:\\3course\\5sem\\OSaSP\\InerprocessCriticalSection\\Debug\\FirstProc.exe");
    std::wstring CommandLineSecond(L"D:\\3course\\5sem\\OSaSP\\InerprocessCriticalSection\\Debug\\SecondProc.exe");
    std::wstring CommandLineThird(L"D:\\3course\\5sem\\OSaSP\\InerprocessCriticalSection\\Debug\\ThirdProc.exe");

    LPWSTR lpwCmdLineFirst = &CommandLineFirst[0];
    LPWSTR lpwCmdLineSecond = &CommandLineSecond[0];
    LPWSTR lpwCmdLineThird = &CommandLineThird[0];

    InitializeCriticalSection(&crSection);

    bufForCrSection = initSomeShit(&mappingForCrSection, sizeOfSection, CRITICAL_SECTION_NAME);
    if (bufForCrSection == NULL) {
        return -1;
    }

    bufForSharedMemory = initSomeShit(&mappingForMemory, sizeOfSharedMemory, SHARED_MEMORY_NAME);
    if (bufForSharedMemory == NULL) {
        freeResources(&mappingForCrSection, &bufForCrSection, &crSection);
        return -1;
    }

    CopyMemory((PVOID)bufForCrSection, &crSection, sizeOfSection);
    CopyMemory((PVOID)bufForSharedMemory, arr, sizeOfSharedMemory);

    BOOL res = CreateProcess(NULL, lpwCmdLineFirst, NULL, NULL, FALSE, 0, NULL, NULL, &siProcFirst, &piProcFirst);
    if (!res) {
        freeResources(&mappingForCrSection, &bufForCrSection, NULL);
        freeResources(&mappingForMemory, &bufForSharedMemory, &crSection);
        return -1;
    }

    res = CreateProcess(NULL, lpwCmdLineSecond, NULL, NULL, FALSE, 0, NULL, NULL, &siProcSecond, &piProcSecond);
    if (!res) {
        freeResources(&mappingForCrSection, &bufForCrSection, NULL);
        freeResources(&mappingForMemory, &bufForSharedMemory, &crSection);
        return -1;
    }

    res = CreateProcess(NULL, lpwCmdLineThird, NULL, NULL, FALSE, 0, NULL, NULL, &siProcThird, &piProcThird);
    if (!res) {
        freeResources(&mappingForCrSection, &bufForCrSection, NULL);
        freeResources(&mappingForMemory, &bufForSharedMemory, &crSection);
        return -1;
    }
   // Sleep(2000);
    bool fl = true;
    //while (fl) {
    //    if (TryEnterCriticalSection(&crSection)) {
    //        for (int i = 0; i < N; i++) {
    //            std::cout << "main read " << ((int*)bufForSharedMemory)[i] << std::endl;
    //            ((int*)bufForSharedMemory)[i] += 5;
    //            std::cout << "main wrote " << ((int*)bufForSharedMemory)[i] << std::endl;
    //        }
    //        LeaveCriticalSection(&crSection);
    //        fl = false;
    //    }    
    //}
 
    WaitForSingleObject(piProcFirst.hProcess, INFINITE);
    WaitForSingleObject(piProcSecond.hProcess, INFINITE);
    WaitForSingleObject(piProcThird.hProcess, INFINITE);

    CloseHandle(piProcFirst.hProcess);
    CloseHandle(piProcFirst.hThread);
    CloseHandle(piProcSecond.hProcess);
    CloseHandle(piProcSecond.hThread);    
    CloseHandle(piProcThird.hProcess);
    CloseHandle(piProcThird.hThread);

    freeResources(&mappingForCrSection, &bufForCrSection, NULL);
    freeResources(&mappingForMemory, &bufForSharedMemory, &crSection);
    return 0;
}

LPCTSTR initSomeShit(HANDLE* mapping, size_t size, LPCWSTR name) {
    *mapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
    if (*mapping == NULL) {
        std::cout << "Failed to create mapping\n";
        return NULL;
    }

    LPCTSTR _pBuf = (LPCTSTR)MapViewOfFile(*mapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (_pBuf == NULL) {
        return NULL;
    }

    return _pBuf;
}

void freeResources(HANDLE* mapping, LPCTSTR* buff,CRITICAL_SECTION* section) {
    if (buff != NULL)
        UnmapViewOfFile(*buff);
    if (mapping != NULL)
        CloseHandle(*mapping);
    if (section != NULL)
        DeleteCriticalSection(section);
}